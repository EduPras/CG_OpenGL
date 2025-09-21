#include "mesh.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include "utils.hpp"

// Constructor initializes all OpenGL handles to 0 and sets default render mode
Mesh::Mesh() : 
    VAO_lines(0), VBO_lines(0), EBO_lines(0), line_index_count(0),
    VAO_points(0), VBO_points(0), point_vertex_count(0),
    currentRenderMode(POINTS) // Default to the DDA points renderer
{}

bool Mesh::loadFromOBJ(const std::string& filename) {
    points = loadOBJPoints(filename);
    face_indices = loadOBJFaces(filename);
    if(points.empty() || face_indices.empty()){
        std::cerr << "Warning: Mesh loading resulted in empty points or faces." << std::endl;
        return false;
    }
    std::cout << "Loaded " << points.size() << " vertices and " << face_indices.size() << " faces from " << filename << std::endl;
    return true;
}

void Mesh::buildHalfEdge() {
    buildHalfEdgeMeshFromPointsAndFaces(points, face_indices, verticesHE, halfedgesHE, facesHE);
}

// Sets the active rendering mode
void Mesh::setRenderMode(RenderMode newMode) {
    currentRenderMode = newMode;
}


// This function is now responsible for setting up the GPU buffers for
// BOTH rendering methods. This is a one-time setup cost.
void Mesh::setupMesh() {
    if (halfedgesHE.empty()) {
        std::cerr << "Cannot setup mesh for rendering: half-edge structure not built." << std::endl;
        return;
    }

    // --- 1. Setup for GL_LINES rendering ---
    {
        std::vector<unsigned int> line_indices;
        // Iterate through all half-edges to find unique edges for drawing
        for (const auto& he : halfedgesHE) {
            // To avoid processing each edge twice (once for each twin),
            // we only process the one with the lower memory address.
            if (he.twin != nullptr && &he > he.twin) {
                continue;
            }
            
            // Get the integer indices of the start and end vertices of the edge
            unsigned int idx1 = he.origin - &verticesHE[0];
            unsigned int idx2 = he.next->origin - &verticesHE[0];
            line_indices.push_back(idx1);
            line_indices.push_back(idx2);
        }
        line_index_count = line_indices.size();

        glGenVertexArrays(1, &VAO_lines);
        glGenBuffers(1, &VBO_lines);
        glGenBuffers(1, &EBO_lines);

        glBindVertexArray(VAO_lines);
        
        // VBO: Contains all vertex data (x, y, z, pointer, etc.)
        glBindBuffer(GL_ARRAY_BUFFER, VBO_lines);
        glBufferData(GL_ARRAY_BUFFER, verticesHE.size() * sizeof(Vertex), verticesHE.data(), GL_STATIC_DRAW);

        // EBO: Contains pairs of indices telling OpenGL which vertices form a line
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_lines);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, line_indices.size() * sizeof(unsigned int), line_indices.data(), GL_STATIC_DRAW);

        // Tell OpenGL how to interpret the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
    }

    // --- 2. Setup for GL_POINTS rendering (using DDA) ---
    {
        std::vector<float> point_cloud_vertices;
        for (const auto& he : halfedgesHE) {
            if (he.twin != nullptr && &he > he.twin) continue;

            Vertex* v1 = he.origin;
            Vertex* v2 = he.next->origin;

            // Generate a list of points along the edge using your DDA function
            auto edge_points = drawSegmentByLineEquation3D({v1->x, v1->y, v1->z}, {v2->x, v2->y, v2->z});
            for(const auto& p : edge_points) {
                point_cloud_vertices.push_back(p.x);
                point_cloud_vertices.push_back(p.y);
                point_cloud_vertices.push_back(p.z);
            }
        }
        point_vertex_count = point_cloud_vertices.size() / 3;

        glGenVertexArrays(1, &VAO_points);
        glGenBuffers(1, &VBO_points);

        glBindVertexArray(VAO_points);
        glBindBuffer(GL_ARRAY_BUFFER, VBO_points);
        glBufferData(GL_ARRAY_BUFFER, point_cloud_vertices.size() * sizeof(float), point_cloud_vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindVertexArray(0);
    }
}

// This function now checks the current rendering mode and issues the appropriate draw call.
void Mesh::draw() {
    if (currentRenderMode == LINES) {
        glBindVertexArray(VAO_lines);
        glDrawElements(GL_LINES, line_index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else { // POINTS
        glBindVertexArray(VAO_points);
        glDrawArrays(GL_POINTS, 0, point_vertex_count);
        glBindVertexArray(0);
    }
}

