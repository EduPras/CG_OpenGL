
#include <string>
#ifndef HALF_EDGE_HPP
#define HALF_EDGE_HPP

#include <vector>
#include <set>

struct Vertex;
struct HalfEdge;
struct Face;

struct Vertex {
    float x, y, z;
    HalfEdge* edge = nullptr;
};

struct HalfEdge {
    Vertex* origin = nullptr;
    HalfEdge* twin = nullptr;
    HalfEdge* next = nullptr;
    Face* face = nullptr;
};

struct Face {
    HalfEdge* edge = nullptr;
};

// Adjacency queries
std::vector<Face*> getAdjacentFacesOfFace(Face* f);
std::vector<Face*> getAdjacentFacesOfEdge(HalfEdge* e);
std::vector<Face*> getFacesOfVertex(Vertex* v);
std::vector<HalfEdge*> getEdgesOfVertex(Vertex* v);
// Build half-edge mesh from loaded points and face indices

void buildHalfEdgeMeshFromPointsAndFaces(
    const std::vector<Point>& points,
    const std::vector<std::vector<int>>& face_indices,
    std::vector<Vertex>& vertices,
    std::vector<HalfEdge>& halfedges,
    std::vector<Face>& faces
);

#endif // HALF_EDGE_HPP
