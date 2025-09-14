
#include <string>
#ifndef HALF_EDGE_HPP
#define HALF_EDGE_HPP

#include <vector>
#include <set>
#include "utils.hpp"

/**
 * @brief Vertex in a half-edge mesh structure.
 *
 * Stores 3D position and a pointer to one outgoing half-edge.
 */
struct Vertex {
    float x, y, z;           ///< 3D position
    HalfEdge* edge = nullptr;///< Pointer to one outgoing half-edge
};

/**
 * @brief Half-edge in a half-edge mesh structure.
 *
 * Stores pointers to origin vertex, twin edge, next edge, and face.
 */
struct HalfEdge {
    Vertex* origin = nullptr;///< Origin vertex
    HalfEdge* twin = nullptr;///< Twin (opposite) half-edge
    HalfEdge* next = nullptr;///< Next half-edge in face
    Face* face = nullptr;    ///< Face this half-edge borders
};

/**
 * @brief Face in a half-edge mesh structure.
 *
 * Stores a pointer to one of its bounding half-edges.
 */
struct Face {
    HalfEdge* edge = nullptr;///< Pointer to one bounding half-edge
};

/**
 * @brief Get all faces adjacent to a given face.
 * @param f Pointer to the face.
 * @return Vector of adjacent face pointers.
 */
std::vector<Face*> getAdjacentFacesOfFace(Face* f);

/**
 * @brief Get all faces adjacent to a given half-edge.
 * @param e Pointer to the half-edge.
 * @return Vector of adjacent face pointers.
 */
std::vector<Face*> getAdjacentFacesOfEdge(HalfEdge* e);

/**
 * @brief Get all faces sharing a given vertex.
 * @param v Pointer to the vertex.
 * @return Vector of face pointers.
 */
std::vector<Face*> getFacesOfVertex(Vertex* v);

/**
 * @brief Get all half-edges originating from a given vertex.
 * @param v Pointer to the vertex.
 * @return Vector of half-edge pointers.
 */
std::vector<HalfEdge*> getEdgesOfVertex(Vertex* v);

/**
 * @brief Build a half-edge mesh from points and face indices.
 *
 * Populates the vertices, halfedges, and faces vectors.
 *
 * @param points List of vertex positions.
 * @param face_indices List of faces (each as a list of vertex indices).
 * @param vertices Output vector of Vertex structs.
 * @param halfedges Output vector of HalfEdge structs.
 * @param faces Output vector of Face structs.
 */
void buildHalfEdgeMeshFromPointsAndFaces(
    const std::vector<Point>& points,
    const std::vector<std::vector<int>>& face_indices,
    std::vector<Vertex>& vertices,
    std::vector<HalfEdge>& halfedges,
    std::vector<Face>& faces
);

#endif // HALF_EDGE_HPP
