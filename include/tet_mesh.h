#pragma once

#include "geometrycentral/surface/simple_polygon_mesh.h"
#include "geometrycentral/surface/surface_mesh.h"


using namespace geometrycentral;
using namespace geometrycentral::surface;

class SimpleTetMesh : public SimplePolygonMesh{
public:
    std::vector<std::vector<size_t>> tets;
    
    SimpleTetMesh(const std::vector<std::vector<size_t>>& tets_,
                  const std::vector<Vector3>& vertexCoordinates_)
                  :SimplePolygonMesh(triangles_from_tets(tets_), vertexCoordinates_), tets(tets_){}
    
    ~SimpleTetMesh(){}

    // ungarbage functions
    void mergeIdenticalVertices(); // overriding parent function; just changing some minor stuff.
    void mergeIdenticalFaces(); // overriding parent function; just changing some minor stuff.
    

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