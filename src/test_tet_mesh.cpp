// Normal curvers

#include "geometrycentral/surface/manifold_surface_mesh.h"
#include "geometrycentral/surface/meshio.h"
#include "geometrycentral/surface/vertex_position_geometry.h"

#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/volume_mesh.h"

#include "tet_mesh.h"
#include "tet_tiling.h"

#include "imgui.h"

#include "colormap.h"

using namespace geometrycentral;
using namespace geometrycentral::surface;

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

EdgeData<int> edge_ints;

// Some global variables
Vector<double> DELTA;                      // sources
polyscope::SurfaceGraphQuantity* currEdge; // currently active vertex
polyscope::SurfaceEdgeScalarQuantity* surface_edge_ints;

polyscope::SurfaceGraphQuantity* normal_curves;
// double maxPhi = 0.0;
double selected_edge_radius;
double normal_curves_graph_radius;


/*
* Display normal curves with uniform vertex placement.
*/
void draw_normal_surfaces(){
    
}

/* 
* finding edge values
* one integer per edge
*/
void assign_edge_ints(){
    edge_ints = EdgeData<int>(*tet_mesh, 0);

    // int c0=0, c1=0, c2=0, c3=0;
    Vertex  v0 = tet_mesh->vertex(0),
            v1 = tet_mesh->vertex(1),
            v2 = tet_mesh->vertex(2),
            v3 = tet_mesh->vertex(3),
            v4 = tet_mesh->vertex(4);
    int c0=0, c1=0, c2=0, c3=0;
    int d01 = 0, d02 = 1, d03 = 1;
    edge_ints[tet_mesh->connectingEdge(v0, v2)] = c0 + c2 + d01 + d03; // (0,2)
    edge_ints[tet_mesh->connectingEdge(v0, v1)] = c0 + c1 + d02 + d03; // (0,1)
    edge_ints[tet_mesh->connectingEdge(v1, v2)] = c1 + c2 + d01 + d02; // (1,2)
    edge_ints[tet_mesh->connectingEdge(v0, v3)] = c0 + c3 + d01 + d02; // (0,3)
    edge_ints[tet_mesh->connectingEdge(v2, v3)] = c3 + c2 + d02 + d03; // (2,3)
    edge_ints[tet_mesh->connectingEdge(v1, v3)] = c1 + c3 + d01 + d03; // (1,3)

    //after double
    int C4 = 0;    
    edge_ints[tet_mesh->connectingEdge(v4, v1)] = C4 + 1; 
    edge_ints[tet_mesh->connectingEdge(v4, v2)] = C4 + 2; 
    edge_ints[tet_mesh->connectingEdge(v4, v3)] = C4 + 0;
    //after edge-flip
    // edge_ints[tet_mesh->connectingEdge(v4, v0)] = 2; 
}

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
    
    // Initalize
    assign_edge_ints();
    surface_edge_ints = psMesh->addEdgeScalarQuantity("cut count", edge_ints, 
                                  polyscope::DataType::MAGNITUDE);
    psMesh->setBackFacePolicy(polyscope::BackFacePolicy::Identical);

    // Give control to the polyscope gui
    polyscope::show();

    delete tet_mesh;
    delete geometry;

    return EXIT_SUCCESS;
}