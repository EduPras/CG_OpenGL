// Add constructor with name
// Mesh::Mesh(const std::string& name_) : name(name_) {}
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "utils.hpp"
#include "xiaolin_wu.hpp"
#include "mesh.hpp"

// Constructor initializes all OpenGL handles to 0 and sets default render mode
Mesh::Mesh(const std::string& name_) : 
    // VAO_lines(0), VBO_lines(0), EBO_lines(0), line_index_count(0),
    // VAO_points(0), VBO_points(0), point_vertex_count(0),
    name(name_),
    currentRenderMode(XIAOLIN_WU), // Default to the DDA points renderer
    shader(nullptr)
{
}

void Mesh::translate(const glm::vec3& trans) {
    modelMatrix = glm::translate(modelMatrix, trans);
}

void Mesh::rotate(float angle, const glm::vec3& axis) {
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
}

void Mesh::scale(const glm::vec3& scaleVec) {
    modelMatrix = glm::scale(modelMatrix, scaleVec);
}

void Mesh::resetTransformations() {
    modelMatrix = glm::mat4(1.0f); // Reset to identity matrix
}

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
// all rendering methods. This is a one-time setup cost.
void Mesh::setupMesh() {
    if (halfedgesHE.empty()) {
        std::cerr << "Cannot setup mesh for rendering: half-edge structure not built." << std::endl;
        return;
    }

    edge_indices.clear();

    for (const auto& he : halfedgesHE) {
        if (he.twin != nullptr && &he > he.twin) {
            continue;
        }
        unsigned int idx1 = he.origin - &verticesHE[0];
        unsigned int idx2 = he.next->origin - &verticesHE[0];
        edge_indices.push_back({idx1, idx2});
    }

    glGenVertexArrays(1, &VAO_wu);
    glGenBuffers(1, &VBO_wu);

    glBindVertexArray(VAO_wu);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_wu);

    // Allocating an initial size for the Wu VBO
    wu_vbo_allocated_size = 2000000;
    glBufferData(GL_ARRAY_BUFFER, wu_vbo_allocated_size * sizeof(WuVertex), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WuVertex), (void*)offsetof(WuVertex, position));
    glEnableVertexAttribArray(0);
    // Color attribute (vec4)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(WuVertex), (void*)offsetof(WuVertex, color));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::drawWithXiaolinWu(Shader* shader,
    const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight) {
    // Clear the CPU-side buffer from the previous frame
    wu_vertex_buffer.clear();
    
    glm::vec4 lineColor = {1.0f, 1.0f, 1.0f, 1.0f}; // White line color

    // Iterate through each edge of the mesh
    for (const auto& edge : edge_indices) {

        // Get the LOCAL 3D positions of the edge's vertices
        glm::vec3 p1_local = {verticesHE[edge.first].x, verticesHE[edge.first].y, verticesHE[edge.first].z};
        glm::vec3 p2_local = {verticesHE[edge.second].x, verticesHE[edge.second].y, verticesHE[edge.second].z};
        
        // Apply the model matrix to get world coordinates
        glm::vec3 p1_world = glm::vec3(model * glm::vec4(p1_local, 1.0f));
        glm::vec3 p2_world = glm::vec3(model * glm::vec4(p2_local, 1.0f));
        
        // Project these world points to 2D screen space
        glm::vec2 p1_screen = projectWorldToScreen(p1_world, view, projection, screenWidth, screenHeight);
        glm::vec2 p2_screen = projectWorldToScreen(p2_world, view, projection, screenWidth, screenHeight);

        // Run Xiaolin Wu's algorithm on the 2D line
        std::vector<Pixel> pixels = drawWuLine2D(p1_screen, p2_screen);
        
        // Convert the pixels into vertices and add to our CPU buffer
        for (const auto& px : pixels) {
            if (px.intensity > 0.01) { // Optional: cull very dim pixels
                WuVertex v;
                v.position = glm::vec2(px.x, px.y);
                v.color = glm::vec4(lineColor.r, lineColor.g, lineColor.b, px.intensity);
                wu_vertex_buffer.push_back(v);
            }
        }
    }
    
    // Update the GPU buffer with the new vertex data for this frame
    if (!wu_vertex_buffer.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_wu);
        
        if (wu_vertex_buffer.size() > wu_vbo_allocated_size) {
            // Reallocate a larger buffer if needed
            wu_vbo_allocated_size = wu_vertex_buffer.size() * 1.5;
            std::cout << "Resizing Wu VBO to " << wu_vbo_allocated_size << " vertices." << std::endl;
            glBufferData(GL_ARRAY_BUFFER, wu_vbo_allocated_size * sizeof(WuVertex), wu_vertex_buffer.data(), GL_DYNAMIC_DRAW);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, wu_vertex_buffer.size() * sizeof(WuVertex), wu_vertex_buffer.data());
        }

        wu_point_count = wu_vertex_buffer.size();
    } else {
        wu_point_count = 0;
    }
    
    // Draw the points
    if (wu_point_count > 0) {
        shader->activate();
        shader->setVec2("u_screenSize", {screenWidth, screenHeight});

        // Enable alpha blending for anti-aliasing
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glDisable(GL_DEPTH_TEST);

        glBindVertexArray(VAO_wu);
        glDrawArrays(GL_POINTS, 0, wu_point_count);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);

        glDisable(GL_BLEND);
    }
}

// void Mesh::draw(glm::mat4& view, glm::mat4& projection, Shader *shader, int screenWidth, int screenHeight) {
//     setShader(shader);
//     if (currentRenderMode == LINES) {
//         glBindVertexArray(VAO_lines);
//         glDrawElements(GL_LINES, line_index_count, GL_UNSIGNED_INT, 0);
//         glBindVertexArray(0);
//     } else if (currentRenderMode == POINTS) {
//         glBindVertexArray(VAO_points);
//         glDrawArrays(GL_POINTS, 0, point_vertex_count);
//         glBindVertexArray(0);
//     }
// }

