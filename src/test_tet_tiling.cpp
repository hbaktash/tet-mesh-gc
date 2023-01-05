// Normal curvers

#include "geometrycentral/volume/manifold_surface_mesh.h"
#include "geometrycentral/volume/meshio.h"
#include "geometrycentral/volume/vertex_position_geometry.h"
#include "geometrycentral/volume/tet_mesh_mutation_manager.h"
#include "geometrycentral/volume/tet_mesh.h"
#include "geometrycentral/utilities/disjoint_sets.h"

#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/volume_mesh.h"

#include "tet_tiling.h"

#include "args/args.hxx"
#include "imgui.h"

#include "colormap.h"

using namespace geometrycentral;
using namespace geometrycentral::volume;

// == Geometry-central data
std::unique_ptr<TetMesh> tet_mesh_uptr;
std::unique_ptr<VertexPositionGeometry> geometry_uptr;
// so we can more easily pass these to different classes
TetMesh* tet_mesh;
VertexPositionGeometry* tet_geometry;

std::unique_ptr<SurfaceMesh> ns_uptr;
std::unique_ptr<VertexPositionGeometry> ns_geometry_uptr;
// so we can more easily pass these to different classes
SurfaceMesh* ns;
VertexPositionGeometry* ns_geometry;

// Polyscope visualization handle, to quickly add data to the surface
int N = 2;
polyscope::SurfaceMesh* psMesh;
polyscope::SurfaceMesh* psTetSkeletonMesh;
polyscope::VolumeMesh* psTetMesh;
std::string MESHNAME = "tet skeleton";

std::vector<std::string> all_edges_items = {std::string("select an edge!")};
std::string all_edges_current_item = "None";
static const char* all_edges_current_item_c_str = "None";
Edge current_edge;

int face_ind = 0, tet_ind = 0, edge_ind = 0;
float raise_size = 1., edge_split_ratio = 0.5;

void redraw_hosting_tet_mesh(){
    if (polyscope::hasSurfaceMesh(MESHNAME)) polyscope::removeSurfaceMesh(MESHNAME);
    psTetSkeletonMesh = polyscope::registerSurfaceMesh(MESHNAME, tet_geometry->inputVertexPositions, tet_mesh->getFaceVertexList(),
                                        polyscopePermutations(*tet_mesh));
    psTetSkeletonMesh->setTransparency(0.3);
    psTetSkeletonMesh->setBackFacePolicy(polyscope::BackFacePolicy::Identical);
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


Vertex buildTetOnFace(Face f, TetMesh &mesh, VertexPositionGeometry &geometry){
    Halfedge fHe = f.halfedge();
    Vertex v1 = fHe.tailVertex(), v2 = fHe.tipVertex(), v3 = fHe.next().tipVertex();
    Vertex v = mesh.buildVolOnFace(f);
    mesh.validateConnectivity();
    std::cout<<"compressed.\n";
    mesh.compress(); // otherwise vector size is doubles and we get lots of meaningless indices
    mesh.compressTets();
    std::cout<<"compressed.\n";
    VertexData<Vector3> newPositions(mesh);
    for(Vertex vv: mesh.vertices()){
        if(vv.getIndex() != v.getIndex()){
            newPositions[vv] = geometry.inputVertexPositions[vv];
        }
    }
    Vector3 baryCenter = (geometry.inputVertexPositions[v1] + geometry.inputVertexPositions[v2] + geometry.inputVertexPositions[v3])/3.;
    Vector3 newPos = geometry.faceNormal(f)*raise_size + baryCenter;
    newPositions[v] = newPos;
    geometry.inputVertexPositions = newPositions;
    geometry.refreshQuantities();
    return v;
}


Vertex splitTet(Tet t, TetMesh &mesh, VertexPositionGeometry &geometry){
    Vector3 baryCenter = {0., 0., 0.};
    for(Vertex v: t.adjVertices()){
        baryCenter += geometry.inputVertexPositions[v];
    }
    baryCenter = baryCenter/4.;

    Vertex v = tet_mesh->splitTet(t);
    // takes time tho
    tet_mesh->validateConnectivity();
    std::cout<<"compressing..\n";
    mesh.compress(); // otherwise vector size is doubles and we get lots of meaningless indices?
    mesh.compressTets();
    std::cout<<"compressed.\n";
    VertexData<Vector3> newPositions(mesh);
    for(Vertex vv: mesh.vertices()){
        if(vv.getIndex() != v.getIndex()){
            newPositions[vv] = geometry.inputVertexPositions[vv];
        }
    }
    newPositions[v] = baryCenter;
    geometry.inputVertexPositions = newPositions;
    geometry.refreshQuantities();
    return v;
}


Vertex splitEdge(Edge e, TetMesh &mesh, VertexPositionGeometry &geometry){
    Vector3 baryCenter = {0., 0., 0.};
    baryCenter += (geometry.inputVertexPositions[e.firstVertex()] + geometry.inputVertexPositions[e.secondVertex()])/2.;
    
    Vertex v = tet_mesh->splitEdge(e);
    tet_mesh->validateConnectivity();
    std::cout<<"compressing..\n";
    mesh.compress(); // otherwise? vector size is doubles and we get lots of meaningless indices?
    mesh.compressTets();
    std::cout<<"compressed.\n";
    VertexData<Vector3> newPositions(mesh);
    for(Vertex vv: mesh.vertices()){
        if(vv.getIndex() != v.getIndex()){
            newPositions[vv] = geometry.inputVertexPositions[vv];
        }
    }
    newPositions[v] = baryCenter;
    geometry.inputVertexPositions = newPositions;
    geometry.refreshQuantities();
    return v;
}

void functionCallback() {
    if (ImGui::Button("some info")) {
        polyscope::warning(" nEdges " + std::to_string(tet_mesh->nEdges()) + 
                           " nVertices " + std::to_string(tet_mesh->nVertices()), 
                           " nFaces " + std::to_string(tet_mesh->nFaces()) + 
                           " nTets " + std::to_string(tet_mesh->nTets()));
    }
    ImGui::SliderInt("Face index", &face_ind, 0, tet_mesh->nFaces()-1);
    ImGui::SliderFloat("raise magnitude", &raise_size, -10., 10.);
    
    if (ImGui::Button("raise face")) {
        Face f = tet_mesh->face(face_ind);
        if(f.isDead()) polyscope::warning(" face with index " + std::to_string(face_ind) + " is dead ", " ");
        else{
            Vertex new_v = buildTetOnFace(f, *tet_mesh, *tet_geometry);
            polyscope::removeVolumeMesh(MESHNAME);
            tet_geometry->requireVertexPositions();
            redraw_hosting_tet_mesh();
        }
    }
    ImGui::SliderInt("Tet index", &tet_ind, 0, tet_mesh->nTets()-1);
    if (ImGui::Button("Split Tet")) {
        Tet t = tet_mesh->tet(tet_ind);
        printf(" tet %d chosen\n", t.getIndex());
        
        Vertex new_v = splitTet(t, *tet_mesh, *tet_geometry);
        printf("the new vertex is %d\n", new_v.getIndex());
        tet_geometry->requireVertexPositions();
        printf("geo locs %d tet neigh counts %d\n", tet_geometry->inputVertexPositions.size(), tet_mesh->tAdjVs.size());
        redraw_hosting_tet_mesh();
    }
    if(ImGui::Button("order siblings")) {
        tet_mesh->order_all_siblings();
        tet_mesh->validateConnectivity();
    }
    ImGui::SliderInt("Edge index", &edge_ind, 0, tet_mesh->nEdges()-1);
    if (ImGui::Button("Split Edge")) {
        Edge e = tet_mesh->edge(edge_ind);
        printf(" Edge %d chosen\n", e.getIndex());
        
        Vertex new_v = splitEdge(e, *tet_mesh, *tet_geometry);
        printf("the new vertex is %d\n", new_v.getIndex());
        tet_geometry->requireVertexPositions();
        redraw_hosting_tet_mesh();
    }
}


int main(int argc, char** argv) {
    // Configure the argument parser
    args::ArgumentParser parser("tet-tiling-test");
    args::Positional<std::string> input_tiling_reso(parser, "resolution", "number of points along each axis.");

    // Parse args
    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    // If a mesh name was not given, use default mesh.
    std::string N_str = "2";
    if (input_tiling_reso) {
        N_str = args::get(input_tiling_reso);
    }

    // make tet mesh
    N = std::stoi(N_str);
    SimplePolygonMesh simple_greedy_tet_mesh = greedy_cube_tiling(N);
    std::cout<<"tet_cnt " << simple_greedy_tet_mesh.nFaces() << " geo cnt " << simple_greedy_tet_mesh.vertexCoordinates.size()<<std::endl;

    std::tie(tet_mesh_uptr, geometry_uptr) = greedy_tiling_to_tet_mesh(simple_greedy_tet_mesh);
    tet_mesh = tet_mesh_uptr.release();
    tet_geometry = geometry_uptr.release();

    // Initialize polyscope
    polyscope::init();

    // Add mesh to GUI
    redraw_hosting_tet_mesh();
    // psTetSkeletonMesh = polyscope::registerSurfaceMesh("greedy tiling", tet_geometry->inputVertexPositions, tet_mesh->getFaceVertexList(),
    //                                         polyscopePermutations(*tet_mesh));

    // psTetMesh = polyscope::registerTetMesh("tet mesh", geometry->inputVertexPositions, tet_mesh->tets);
    
    // Initalize
    Vector3 centroid{0.,0.,0.};
    for(Vertex v: tet_mesh->vertices()){
        centroid += tet_geometry->inputVertexPositions[v]/((double)tet_mesh->nVertices());
    }
    
    current_edge = tet_mesh->edge(0); // to avoid crashing combo lists
    // Set the callback function
    printf("first connectivity check!\n");
    tet_mesh->validateConnectivity();
    printf("ordering siblings!\n");
    tet_mesh->order_all_siblings();
    printf("second connectivity check!\n");
    tet_mesh->validateConnectivity();
    polyscope::state::userCallback = functionCallback;
    // Give control to the polyscope gui
    polyscope::show();

    delete tet_mesh;
    delete tet_geometry;

    return EXIT_SUCCESS;
}