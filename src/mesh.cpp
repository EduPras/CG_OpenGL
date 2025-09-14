#include "../include/mesh.hpp"
#include "../include/utils.hpp"
#include "../include/half_edge.hpp"

bool Mesh::loadFromOBJ(const std::string& filename) {
    points = loadOBJPoints(filename);
    faces = loadOBJFaces(filename);
    return !points.empty() && !faces.empty();
}

void Mesh::buildHalfEdge() {
    buildHalfEdgeMeshFromPointsAndFaces(points, faces, verticesHE, halfedgesHE, facesHE);
}
