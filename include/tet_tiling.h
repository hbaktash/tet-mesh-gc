#pragma once

#include "geometrycentral/surface/simple_polygon_mesh.h"
#include "geometrycentral/surface/surface_mesh.h"
#include "geometrycentral/surface/vertex_position_geometry.h"
#include "tet_mesh.h"

using namespace geometrycentral;
using namespace geometrycentral::surface;



// generate tilings
// unit cube with N^3 cubes
SimplePolygonMesh greedy_cube_tiling(int N);

// garbage to very non-manifold surface mesh + geometry
std::tuple<std::unique_ptr<TetMesh>, std::unique_ptr<VertexPositionGeometry>> 
greedy_tiling_to_tet_mesh(SimplePolygonMesh& greedy_tet_tiling);