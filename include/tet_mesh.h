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
    Face get_connecting_face(Vertex v1, Vertex v2, Vertex v3); // assuming we dont have non-manifold(?!) edges or duplicat(!?) or.. faces (3-manifold) ; generally we should return a vector<Face>.
    Tet get_connecting_tet(Vertex v1, Vertex v2, Vertex v3, Vertex v4); // assuming we dont have duplicate(?!) tets..; otherwise ..//..
    
    // Range-based loops
    TetSet tets();

    // element counters getters
    size_t nTets() const;
    size_t nTetsCapacity() const;

protected:
    // element counters
    size_t nTetsCount = 0;
    size_t nTetsCapacityCount = 0;
};

}
}

#include <tet_mesh.ipp>