#pragma once

#include <vector>
#include <string>
// #include "utils.hpp"
#include "half_edge.hpp"

class Mesh {
public:
    // Enum to define the available rendering modes
    enum RenderMode {
        LINES,
        POINTS
    };

    // Raw data loaded from the OBJ file
    std::vector<Point> points;
    std::vector<std::vector<int>> face_indices;

    // Half-edge data structures
    std::vector<Vertex> verticesHE;
    std::vector<HalfEdge> halfedgesHE;
    std::vector<Face> facesHE;

    // --- UPDATED: OpenGL handles for both rendering modes ---
    // For GL_LINES rendering
    unsigned int VAO_lines, VBO_lines, EBO_lines;
    unsigned int line_index_count;
    // For GL_POINTS rendering (using DDA)
    unsigned int VAO_points, VBO_points;
    unsigned int point_vertex_count;

    // The currently active rendering mode
    RenderMode currentRenderMode;

    Mesh();
    bool loadFromOBJ(const std::string& filename);
    void buildHalfEdge();
    
    // --- UPDATED: Core mesh functions ---
    // This now prepares the buffers for BOTH rendering modes
    void setupMesh(); 
    // This now checks the current mode and draws accordingly
    void draw();      
    // Use this function to change the rendering mode
    void setRenderMode(RenderMode newMode);
};
