#include "tet_tiling.h"


SimplePolygonMesh greedy_cube_tiling(int N){
    // number of vertices is N-1; assuming each vertex is (0,0,0) corner of a cube.
    /* 
       7_________6
       /|       /|
     4/_|_____5/ |
      | |3_____|_|2
      | /      | /
      |/______ |/
      0       1
    */
    double d = 1./((double)N);
    size_t vCnt = 0;
    std::vector<std::vector<size_t>> tets;
    std::vector<Vector3> vertex_coordinates;
    Vector3 dx = {d, 0., 0.}, 
            dy = {0., d, 0.},
            dz = {0., 0., d};
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            for(int k = 0; k < N; k++){
                Vector3 v0 = {(double)i*d, (double)j*d, (double)k*d};
                Vector3 v1 = v0 + dx, v2 = v0 + dx + dy, v3 = v0 + dy,
                        v4 = v0 + dz, v5 = v0 + dz + dx, v7 = v0 + dz + dy,
                        v6 = v0 + dx + dy + dz;
                vertex_coordinates.push_back(v0);
                vertex_coordinates.push_back(v1);
                vertex_coordinates.push_back(v2);
                vertex_coordinates.push_back(v3);
                vertex_coordinates.push_back(v4);
                vertex_coordinates.push_back(v5);
                vertex_coordinates.push_back(v6);
                vertex_coordinates.push_back(v7);
                tets.push_back({vCnt, vCnt + 1, vCnt + 4, vCnt + 3});
                tets.push_back({vCnt + 1, vCnt + 3, vCnt + 7, vCnt + 4});
                tets.push_back({vCnt + 1, vCnt + 7, vCnt + 4, vCnt + 5});
                tets.push_back({vCnt + 1, vCnt + 3, vCnt + 7, vCnt + 2});
                tets.push_back({vCnt + 1, vCnt + 2, vCnt + 5, vCnt + 7});
                tets.push_back({vCnt + 6, vCnt + 5, vCnt + 2, vCnt + 7});
                vCnt += 8;
            }
        }
    }
    SimplePolygonMesh garbage_simple_tet_mesh(tets, vertex_coordinates); // actually abusing polygons in SimplPMesh here, 
    garbage_simple_tet_mesh.mergeIdenticalVertices();
    return garbage_simple_tet_mesh;
}

// simple mesh to 
std::tuple<std::unique_ptr<TetMesh>, std::unique_ptr<VertexPositionGeometry>> 
greedy_tiling_to_tet_mesh(SimplePolygonMesh& greedy_tet_tiling){
    SimpleTetMesh simpleTetMesh(greedy_tet_tiling.polygons, greedy_tet_tiling.vertexCoordinates);
    simpleTetMesh.mergeIdenticalVertices(); // redundant if coming from my "greedy_cube_tiling()"
    simpleTetMesh.mergeIdenticalFaces();
    std::unique_ptr<TetMesh> tet_mesh;
    tet_mesh.reset(new TetMesh(simpleTetMesh.tets, simpleTetMesh.polygons));
    std::unique_ptr<VertexPositionGeometry> geometry(new VertexPositionGeometry(*tet_mesh));
    for (Vertex v : tet_mesh->vertices()) {
        // Use the low-level indexers here since we're constructing
        (*geometry).inputVertexPositions[v] = greedy_tet_tiling.vertexCoordinates[v.getIndex()];
    }
    return std::make_tuple(std::move(tet_mesh), std::move(geometry));
}