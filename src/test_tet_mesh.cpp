// Normal curvers

#include "geometrycentral/volume/manifold_surface_mesh.h"
#include "geometrycentral/volume/meshio.h"
#include "geometrycentral/volume/vertex_position_geometry.h"
#include "geometrycentral/volume/tet_mesh.h"

#include "polyscope/polyscope.h"
#include "polyscope/surface_mesh.h"
#include "polyscope/volume_mesh.h"
#include "args/args.hxx"

#include "tet_tiling.h"

#include "imgui.h"

#include "colormap.h"

using namespace geometrycentral;
using namespace geometrycentral::volume;

// == Geometry-central data
std::unique_ptr<TetMesh> tet_mesh_uptr;
std::unique_ptr<VertexPositionGeometry> tet_geometry_uptr;
// so we can more easily pass these to different classes
TetMesh* tet_mesh;
VertexPositionGeometry* tet_geometry;

// Polyscope visualization handle, to quickly add data to the surface
int N = 2;
polyscope::SurfaceMesh* psMesh;
polyscope::SurfaceMesh* psTetSkeletonMesh;
polyscope::VolumeMesh* psTetMesh;
std::string MESHNAME;

int face_ind = 0, tet_ind = 0, edge_ind = 0;
float raise_size = 1.;


void redraw_hosting_tet_mesh(){
    if (polyscope::hasSurfaceMesh(MESHNAME)) polyscope::removeSurfaceMesh(MESHNAME);
    psTetSkeletonMesh = polyscope::registerSurfaceMesh(MESHNAME, tet_geometry->inputVertexPositions, tet_mesh->getFaceVertexList(),
                                        polyscopePermutations(*tet_mesh));
    psTetSkeletonMesh->setTransparency(0.3);
    psTetSkeletonMesh->setBackFacePolicy(polyscope::BackFacePolicy::Identical);
}


void showSelected() {}

void showEdge(Vertex v1, Vertex v2) {

}

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
        polyscope::warning(" nEdges: " + std::to_string(tet_mesh->nEdges()) + 
                           " nVertices: " + std::to_string(tet_mesh->nVertices()), 
                           " nFaces: " + std::to_string(tet_mesh->nFaces()) + 
                           " nTets: " + std::to_string(tet_mesh->nTets()));
    }
    ImGui::SliderInt("Face index", &face_ind, 0, tet_mesh->nFaces()-1);
    ImGui::SliderFloat("raise magnitude", &raise_size, -2., 2.);
    
    if (ImGui::Button("raise face")) {
        Face f = tet_mesh->face(face_ind);
        if(f.isDead()) polyscope::warning(" face with index " + std::to_string(face_ind) + " is dead ", " ");
        else{
            Vertex new_v = buildTetOnFace(f, *tet_mesh, *tet_geometry);
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
        redraw_hosting_tet_mesh();
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
    
    // Configure the argument parser
    // args::ArgumentParser parser("tet mesh test");
    // args::Positional<std::string> v1_parser(parser, "1", "vertex1 ");
    // args::Positional<std::string> v2_parser(parser, "2", "vertex2 ");

    // // Parse args
    // try {
    //     parser.ParseCLI(argc, argv);
    // } catch (args::Help) {
    //     std::cout << parser;
    //     return 0;
    // } catch (args::ParseError e) {
    //     std::cerr << e.what() << std::endl;
    //     std::cerr << parser;
    //     return 1;
    // }

    // // If a mesh name was not given, use default mesh.
    // std::string v1_str = "1", v2_str = "2";
    // if (v1_parser) v1_str = args::get(v1_parser);
    // if (v2_parser) v2_str = args::get(v2_parser);
    // size_t v1_ind = std::stoi(v1_str), v2_ind = std::stoi(v2_str);

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
        // {0.5, 0.5,-1.}, // down
        // {1.0, 1.0, 1.} 
    };
    tets = {
        {0, 1, 2, 3},
        // {1, 2, 3, 4},
        // {0, 2, 3, 5}
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
    std::tie(tet_mesh_uptr, tet_geometry_uptr) = greedy_tiling_to_tet_mesh(simple_greedy_tet_mesh);
    tet_mesh = tet_mesh_uptr.release();
    tet_geometry = tet_geometry_uptr.release();

    std::cout<<"initial validating connectivity\n ";
    tet_mesh->validateConnectivity();
    
    std::cout<<"tet_cnt " << simple_greedy_tet_mesh.nFaces() << " geo cnt " << simple_greedy_tet_mesh.vertexCoordinates.size()<<std::endl;
    // geometry->normalize(Vector3::constant(0.), true);
    
    // Initialize polyscope
    polyscope::init();
    // Set the callback function
    polyscope::state::userCallback = functionCallback;
    // Add mesh to GUI
    // psMesh = polyscope::registerSurfaceMesh("dummy tet", geometry->inputVertexPositions, tet_mesh->getFaceVertexList(),
    //                                         polyscopePermutations(*tet_mesh));
    MESHNAME = "tet mesh";
    redraw_hosting_tet_mesh();
    std::cout<<"tets count " << tet_mesh->nTets() << std::endl;
    tet_mesh->order_all_siblings();
    tet_mesh->validateConnectivity();
    // psMesh->setBackFacePolicy(polyscope::BackFacePolicy::Identical);
    // Give control to the polyscope gui
    polyscope::show();
    
    std::string v1_str = "1", v2_str = "2";
    
    delete tet_mesh;
    delete tet_geometry;

    return EXIT_SUCCESS;
}