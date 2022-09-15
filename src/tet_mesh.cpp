#include "tet_mesh.h"
// #include "set"

namespace geometrycentral{
namespace volume{

SimpleTetMesh::SimpleTetMesh(const std::vector<std::vector<size_t>>& tets_, const std::vector<Vector3>& vertexCoordinates_)
    :SimplePolygonMesh(triangles_from_tets(tets_), vertexCoordinates_), tets(tets_){}

void SimpleTetMesh::mergeIdenticalVertices(){
  std::vector<Vector3> compressedPositions;
  // Store mapping from original vertex index to merged vertex index
  std::vector<size_t> compressVertex;
  compressVertex.reserve(vertexCoordinates.size());

  std::unordered_map<Vector3, size_t> canonicalIndex;

  for (size_t iV = 0; iV < vertexCoordinates.size(); ++iV) {
    Vector3 v = vertexCoordinates[iV];
    auto it = canonicalIndex.find(v);

    // Check if vertex exists in map or not
    if (it == canonicalIndex.end()) {
      compressedPositions.push_back(v);
      size_t vecIndex = compressedPositions.size() - 1;
      canonicalIndex[v] = vecIndex;
      compressVertex.push_back(vecIndex);
    } else {
      size_t vecIndex = it->second;
      compressVertex.push_back(vecIndex);
    }
  }

  vertexCoordinates = std::move(compressedPositions);

  // Update face indices.
  for (std::vector<size_t>& face : polygons) {
    for (size_t& iV : face) {
      iV = compressVertex[iV];
    }
  }
  // update tet indices.
  for (std::vector<size_t>& tet : tets) {
    for (size_t& iV : tet) {
      iV = compressVertex[iV];
    }
  }
}

void SimpleTetMesh::mergeIdenticalFaces(){
    std::unordered_set<Vector3> triangle_set; // Could have a int type Vector3 kind of thing to avoid the double casts. Tuple is not Hashable?
    for(std::vector<size_t> triangle: polygons){
        size_t min = *std::min_element(triangle.begin(), triangle.end()), 
               max = *std::max_element(triangle.begin(), triangle.end()), 
               mid = triangle[0] + triangle[1] + triangle[2] - max - min;
        Vector3 t0{(double)min, (double)mid, (double)max}; // sorry
        triangle_set.insert(t0);
    }
    std::vector<std::vector<size_t>> compressed_triangles;
    compressed_triangles.reserve(triangle_set.size());
    
    //compressing triangle vector set
    for (auto itr = triangle_set.begin(); itr != triangle_set.end(); ++itr) {
        Vector3 tri_vec3 = (*itr);
        std::vector<size_t> tri_vec = {(size_t)tri_vec3[0], (size_t)tri_vec3[1], (size_t)tri_vec3[2]};
        compressed_triangles.push_back(tri_vec);
    }
    polygons = std::vector<std::vector<size_t>>(compressed_triangles);
}


TetMesh::TetMesh(const std::vector<std::vector<size_t>>& tets_,
                 const std::vector<std::vector<size_t>>& triangles_)
      :SurfaceMesh(triangles_) {
  // The nonmanifold surface skeleton is already constructed.
  // Faces, vertices, halfedges and edges are initiated.
  tet_objects.reserve(tets_.size());
  size_t tet_ind = 0;
  for(std::vector<size_t> tet_v_inds: tets_){
    Tet new_tet(this, tet_ind++,
                {vertex(tet_v_inds[0]), vertex(tet_v_inds[1]), vertex(tet_v_inds[2]), vertex(tet_v_inds[3])});
    //populating the lazy/dirty iterators on Tets
    new_tet.buildAdjEdges();
    new_tet.buildAdjFaces();

    // populating the lazy/dirty to-Tet iterators on V/E/F's
    for(size_t v_ind:tet_v_inds){
      // --Vertices
      Vertex tmp_v = vertex(v_ind);
      tmp_v.adjTets.push_back(new_tet);

      // --Faces
      // TODO: should encapsulate these set operations in utils :'(
      std::vector<Vertex> triplet; // instead, we should have some set operations added to utils.
      std::vector<Vertex> boring_solo_set{tmp_v};
      triplet.reserve(3);
      std::set_difference(new_tet.adjVertices.begin(), new_tet.adjVertices.end(),  // fancy diff
                          boring_solo_set.begin(), boring_solo_set.end(),
                          std::inserter(triplet, triplet.begin()));
      Face tmp_f = get_connecting_face(triplet[0], triplet[1], triplet[2]);
      tmp_f.adjTets.push_back(new_tet);

      // --Edges
      for(size_t other_v_ind: tet_v_inds){
        if(v_ind != other_v_ind){
          Edge tmp_e = connectingEdge(vertex(v_ind), vertex(other_v_ind));
          if(std::find(tmp_e.adjTets.begin(), tmp_e.adjTets.end(), new_tet) == tmp_e.adjTets.end()){ // so it does not already exist
            tmp_e.adjTets.push_back(new_tet);
          }
        }
      }
    }
    // Tet object is ready to be pushed in. Was actually ready even before handling the elem.adjT iterators; anyway..
    tet_objects.push_back(new_tet);
  }
}

Face TetMesh::get_connecting_face(Vertex v1, Vertex v2, Vertex v3){
  Edge e1 = connectingEdge(v1, v2);
  Edge e2 = connectingEdge(v1, v3);
  
  for(Face f1: e1.adjacentFaces()){
    for(Face f2: e2.adjacentFaces()){
      if(f1 == f2){
        return f1;
      }
    }
  }
  return Face();
}


Tet TetMesh::get_connecting_tet(Vertex v1, Vertex v2, Vertex v3, Vertex v4){
  Face f1 = get_connecting_face(v1, v2, v3);
  Face f2 = get_connecting_face(v1, v2, v4);

  for(Tet t1: f1.adjTets){
    for(Tet t2: f2.adjTets){
      if(t1 == t2){
        return t1;
      }
    }
  }
  return Tet();
}


// helper fucntions 
std::vector<std::vector<size_t>> triangles_from_tets(std::vector<std::vector<size_t>> tets_){
    // just adding tet faces. Will compress later
    int face_cnt = 0;
    std::vector<std::vector<size_t>> triangles;
    triangles.reserve(tets_.size()*4);
    for(auto tet: tets_){
        triangles.push_back({tet[0], tet[1], tet[2]});
        triangles.push_back({tet[1], tet[2], tet[3]});
        triangles.push_back({tet[0], tet[1], tet[3]});
        triangles.push_back({tet[0], tet[2], tet[3]});
        face_cnt += 4;
    }
    
    //compressing in the case of bounded volume; which always happens
    std::vector<std::vector<size_t>> compressed_triangles;
    compressed_triangles.reserve(face_cnt);
    
    for (std::vector<size_t> triangle: triangles) {
        compressed_triangles.push_back(triangle);
    }
    return compressed_triangles;
}


} // namespace volume
} // namespace 