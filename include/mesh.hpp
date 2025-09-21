
#ifndef MESH_HPP
#define MESH_HPP
#pragma once

#include <vector>
#include <string>
#include "utils.hpp"
#include "half_edge.hpp"

class Mesh {
public:
    // Raw data loaded from the OBJ file
    std::vector<Point> points;
    std::vector<std::vector<int>> face_indices;

    // Half-edge data structures
    std::vector<Vertex> verticesHE;
    std::vector<HalfEdge> halfedgesHE;
    std::vector<Face> facesHE;

    // --- NEW: Modern OpenGL rendering members ---
    unsigned int VAO, VBO, EBO; // Vertex Array, Vertex Buffer, Element Buffer Objects
    unsigned int line_index_count; // How many indices to draw for lines

    // --- Member function declarations ---
    Mesh(); // Constructor
    ~Mesh(); // Destructor to clean up GPU resources

    bool loadFromOBJ(const std::string& filename);
    void buildHalfEdge();

    // --- NEW: OpenGL setup and drawing ---
    void setupMesh();
    void draw(); // The new draw call
};

#endif // MESH_HPP
