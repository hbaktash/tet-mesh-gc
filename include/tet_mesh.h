#pragma once

#include "geometrycentral/volume/simple_polygon_mesh.h"
#include "geometrycentral/volume/surface_mesh.h"


namespace geometrycentral{
namespace volume{
// using namespace geometrycentral::surface::volume;

template <typename T>
using TetData = MeshData<Tet, T>;

class SimpleTetMesh : public SimplePolygonMesh{
public:
    std::vector<std::vector<size_t>> tets;
    std::vector<Tet> tet_objects;
    
    SimpleTetMesh(const std::vector<std::vector<size_t>>& tets_,
                  const std::vector<Vector3>& vertexCoordinates_)
                  :SimplePolygonMesh(triangles_from_tets(tets_), vertexCoordinates_), tets(tets_){}
    
    ~SimpleTetMesh(){}


    //counters
    size_t nTetsCapacityCount = 0;
    size_t nTetsFillCount = 0;
    
    //generators
    Tet getNewTet();
    // merging functions
    void mergeIdenticalVertices(); // overriding parent function; just changing some minor stuff.
    void mergeIdenticalFaces(); // merging faces with the same set of vertices (by index).
    

    // helper functions
    std::vector<std::vector<size_t>> triangles_from_tets(std::vector<std::vector<size_t>> tets);
};

class TetMesh : public SurfaceMesh{
public:
    std::vector<std::vector<size_t>> tets;
    
    TetMesh(const std::vector<std::vector<size_t>>& tets_,
            const std::vector<std::vector<size_t>>& triangles_)
            :SurfaceMesh(triangles_), tets(tets_){}
    
    ~TetMesh(){}

    // can have cleaner/faster navigators later.
    std::vector<size_t> tet_neighs_of_edge(Edge e);
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