#include "../include/mesh.hpp"
#include "../include/utils.hpp"
#include "../include/half_edge.hpp"


// Load mesh geometry from an OBJ file: fills points and faces vectors
bool Mesh::loadFromOBJ(const std::string& filename) {
    points = loadOBJPoints(filename);   // Load vertex positions
    faces = loadOBJFaces(filename);     // Load face indices
    // Return true if both points and faces were loaded
    return !points.empty() && !faces.empty();
}
// Build the half-edge mesh structure from points and faces
void Mesh::buildHalfEdge() {
    // This function populates the half-edge data structures for advanced mesh operations
    buildHalfEdgeMeshFromPointsAndFaces(points, faces, verticesHE, halfedgesHE, facesHE);
}
