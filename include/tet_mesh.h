#pragma once

#include "geometrycentral/volume/simple_polygon_mesh.h"
#include "geometrycentral/volume/surface_mesh.h"


namespace geometrycentral{
namespace volume{
// using namespace geometrycentral::surface::volume;

//container template
template <typename T>
using TetData = MeshData<Tet, T>;

// helper functions
std::vector<std::vector<size_t>> triangles_from_tets(std::vector<std::vector<size_t>> tets);

// classes
class SimpleTetMesh : public SimplePolygonMesh{
public:
    std::vector<std::vector<size_t>> tets;
    
    SimpleTetMesh(const std::vector<std::vector<size_t>>& tets_,
                  const std::vector<Vector3>& vertexCoordinates_);
    
    ~SimpleTetMesh(){}


    //counters
    size_t nTetsCapacityCount = 0;
    size_t nTetsFillCount = 0;
    
    //generators
    Tet getNewTet();
    // merging functions
    void mergeIdenticalVertices(); // overriding parent function; just changing some minor stuff.
    void mergeIdenticalFaces(); // merging faces with the same set of vertices (by index).
    

};

class TetMesh : public SurfaceMesh{
public:
    std::vector<std::vector<size_t>> tets;
    std::vector<Tet> tet_objects;


    TetMesh(const std::vector<std::vector<size_t>>& tets_)
            :TetMesh(tets_, triangles_from_tets(tets_)){}
            
    TetMesh(const std::vector<std::vector<size_t>>& tets_,
            const std::vector<std::vector<size_t>>& triangles_); // assuming there are no identical triangles or vertices
    
    ~TetMesh(){}

    // navigation helpers
    Face get_connecting_face(Vertex v1, Vertex v2, Vertex v3); // assumes we dont have non-manifold(?!) or intrinsic(!?) or.. faces in the 3-manifold; generally we should return a vector<Face>.
    Tet get_connecting_tet(Vertex v1, Vertex v2, Vertex v3, Vertex v4); // assumes we dont have duplicate(?!) tets..; otherwise ..//..
    
    // element counters
    size_t nTets() const;
    size_t nTetsCapacity() const;
    TetSet tets();
protected:
    
};

}
}

// //hash function
// struct hashFunction 
// {
//   size_t operator()(const std::array<size_t, 3> &myArray) const {
//     std::hash<size_t> hasher;
//     size_t answer = 0;
      
//     for (size_t i : myArray) {
//       answer ^= hasher(i) + 0x9e3779b9 + 
//                 (answer << 6) + (answer >> 2);
//     }
//     return answer;
//   }
// };