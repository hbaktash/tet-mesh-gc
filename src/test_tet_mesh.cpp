// Normal curvers

#include "geometrycentral/volume/manifold_surface_mesh.h"
#include "geometrycentral/volume/meshio.h"
#include "geometrycentral/volume/vertex_position_geometry.h"
#include "geometrycentral/volume/tet_mesh.h"

#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/volume_mesh.h"

#include "tet_tiling.h"

#include "imgui.h"

#include "colormap.h"

using namespace geometrycentral;
using namespace geometrycentral::volume;

// == Geometry-central data
std::unique_ptr<TetMesh> tet_mesh_uptr;
std::unique_ptr<VertexPositionGeometry> geometry_uptr;
// so we can more easily pass these to different classes
TetMesh* tet_mesh;
VertexPositionGeometry* geometry;

std::unique_ptr<SurfaceMesh> ns_uptr;
std::unique_ptr<VertexPositionGeometry> ns_geometry_uptr;
// so we can more easily pass these to different classes
SurfaceMesh* ns;
VertexPositionGeometry* ns_geometry;

// Polyscope visualization handle, to quickly add data to the surface
int N = 2;
polyscope::SurfaceMesh* psMesh;
polyscope::VolumeMesh* psTetMesh;
std::string MESHNAME;

volume::EdgeData<int> edge_ints;

// Some global variables
Vector<double> DELTA;                      // sources
polyscope::SurfaceGraphQuantity* currEdge; // currently active vertex
polyscope::SurfaceEdgeScalarQuantity* surface_edge_ints;

polyscope::SurfaceGraphQuantity* normal_curves;
// double maxPhi = 0.0;
double selected_edge_radius;
double normal_curves_graph_radius;


/*
 * Show selected edge.
 * This function gets called every time an element is selected on-screen.
 */
void showSelected() {}

void redraw() {
    showSelected();
    polyscope::requestRedraw();
}


void functionCallback() {
    if (ImGui::Button("some info")) {
        polyscope::warning(" nEdges: " + std::to_string(tet_mesh->nEdges()) + 
                           " nVertices: " + std::to_string(tet_mesh->nVertices()), 
                           " nFaces: " + std::to_string(tet_mesh->nFaces()));
    }
}


int main(int argc, char** argv) {
    
    //for this test I will manually make a few tets.
    std::vector<Vector3> positions;
    std::vector<std::vector<size_t>> tets;
    // positions = { // single
    //     {0 ,  0,  0},
    //     {0.5, 1,  0},
    //     {1 ,  0,  0},
    //     {0.5, 0.5,1}
    // };
    // tets = {
    //     {0, 1, 2, 3}
    // };
    positions = { // double
        {0.5, 0.5, 1.}, // up
        {0. ,  0.,  0.},
        {0.5, 1.,  0.},
        {1. ,  0.,  0.},
        {0.5, 0.5,-1.} // down
    };
    tets = {
        {0, 1, 2, 3},
        {1, 2, 3, 4}
    };
    // positions = { // double after delauny
    //     {0.5, 0.5, 1.}, // up
    //     {0. ,  0.,  0.},
    //     {0.5, 1.,  0.},
    //     {1. ,  0.,  0.},
    //     {0.5, 0.5,-1.} // down
    // };
    // tets = {
    //     {0, 1, 2, 4},
    //     {0, 1, 3, 4},
    //     {0, 2, 3, 4}
    // };

    SimplePolygonMesh simple_greedy_tet_mesh(tets, positions);
    simple_greedy_tet_mesh.mergeIdenticalVertices();
    std::tie(tet_mesh_uptr, geometry_uptr) = greedy_tiling_to_tet_mesh(simple_greedy_tet_mesh);
    tet_mesh = tet_mesh_uptr.release();
    geometry = geometry_uptr.release();

    std::cout<<"tet_cnt " << simple_greedy_tet_mesh.nFaces() << " geo cnt " << simple_greedy_tet_mesh.vertexCoordinates.size()<<std::endl;
    
    // Initialize polyscope
    polyscope::init();

    // Set the callback function
    polyscope::state::userCallback = functionCallback;

    // geometry->normalize(Vector3::constant(0.), true);
    // Add mesh to GUI
    psMesh = polyscope::registerSurfaceMesh("dummy tet", geometry->inputVertexPositions, tet_mesh->getFaceVertexList(),
                                            polyscopePermutations(*tet_mesh));

    // psTetMesh = polyscope::registerTetMesh("tet mesh", geometry->inputVertexPositions, tet_mesh->tets);
    //     psMesh->setBackFacePolicy(polyscope::BackFacePolicy::Identical);

    // Give control to the polyscope gui
    polyscope::show();

    delete tet_mesh;
    delete geometry;

    return EXIT_SUCCESS;
}