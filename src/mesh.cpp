#include "mesh.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "utils.hpp"
#include "xiaolin_wu.hpp"

// Constructor initializes all OpenGL handles to 0 and sets default render mode
Mesh::Mesh() : 
    VAO_lines(0), VBO_lines(0), EBO_lines(0), line_index_count(0),
    VAO_points(0), VBO_points(0), point_vertex_count(0),
    currentRenderMode(POINTS), // Default to the DDA points renderer
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

    {
        edge_indices.clear();

        // The logic here is identical to your GL_LINES setup, but instead of
        // sending indices to an EBO, we store them in our own vector.
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

        // We don't load any data yet. We just allocate a large enough buffer
        // and specify that its contents will change frequently (GL_DYNAMIC_DRAW).
        // Let's allocate space for a generous number of points (e.g., 2 million vertices).
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
}

void Mesh::drawWithXiaolinWu(Shader* shader,
    const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight) {
    // Clear the CPU-side buffer from the previous frame
    wu_vertex_buffer.clear();
    
    glm::vec4 lineColor = {1.0f, 1.0f, 1.0f, 1.0f}; // White line color

    // 1. Iterate through each edge of the mesh
    for (const auto& edge : edge_indices) {

        // Get the LOCAL 3D positions of the edge's vertices
        glm::vec3 p1_local = {verticesHE[edge.first].x, verticesHE[edge.first].y, verticesHE[edge.first].z};
        glm::vec3 p2_local = {verticesHE[edge.second].x, verticesHE[edge.second].y, verticesHE[edge.second].z};
        
        // APPLY THE MODEL MATRIX HERE to get world coordinates
        glm::vec3 p1_world = glm::vec3(model * glm::vec4(p1_local, 1.0f));
        glm::vec3 p2_world = glm::vec3(model * glm::vec4(p2_local, 1.0f));
        
        // 2. Project these WORLD points to 2D screen space
        glm::vec2 p1_screen = projectWorldToScreen(p1_world, view, projection, screenWidth, screenHeight);
        glm::vec2 p2_screen = projectWorldToScreen(p2_world, view, projection, screenWidth, screenHeight);

        // 3. Run Xiaolin Wu's algorithm on the 2D line
        std::vector<Pixel> pixels = drawWuLine2D(p1_screen, p2_screen);
        
        // 4. Convert the pixels into vertices and add to our CPU buffer
        for (const auto& px : pixels) {
            if (px.intensity > 0.01) { // Optional: cull very dim pixels
                WuVertex v;
                v.position = glm::vec2(px.x, px.y);
                v.color = glm::vec4(lineColor.r, lineColor.g, lineColor.b, px.intensity);
                wu_vertex_buffer.push_back(v);
            }
        }
    }
    
    // 5. Update the GPU buffer with the new vertex data for this frame
    if (!wu_vertex_buffer.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_wu);
        
        // ---  THE FIX: CHECK IF THE BUFFER IS LARGE ENOUGH ---
        if (wu_vertex_buffer.size() > wu_vbo_allocated_size) {
            // The new data is too big! Re-allocate the VBO.
            // We'll make it 1.5x the required size to avoid re-allocating every frame.
            wu_vbo_allocated_size = wu_vertex_buffer.size() * 1.5;
            std::cout << "Resizing Wu VBO to " << wu_vbo_allocated_size << " vertices." << std::endl;
            glBufferData(GL_ARRAY_BUFFER, wu_vbo_allocated_size * sizeof(WuVertex), wu_vertex_buffer.data(), GL_DYNAMIC_DRAW);
        } else {
            // The data fits, so just update the existing buffer (this is faster).
            glBufferSubData(GL_ARRAY_BUFFER, 0, wu_vertex_buffer.size() * sizeof(WuVertex), wu_vertex_buffer.data());
        }

        wu_point_count = wu_vertex_buffer.size();
    } else {
        wu_point_count = 0;
    }
    
    // 6. Draw the points
    if (wu_point_count > 0) {
        // Use your simple pass-through shader for drawing points
        // (The one from the previous answer)
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

// This function now checks the current rendering mode and issues the appropriate draw call.
void Mesh::draw(glm::mat4& view, glm::mat4& projection, Shader *shader, int screenWidth, int screenHeight) {
    setShader(shader);
    if (currentRenderMode == LINES) {
        glBindVertexArray(VAO_lines);
        glDrawElements(GL_LINES, line_index_count, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    } else if (currentRenderMode == POINTS) {
        glBindVertexArray(VAO_points);
        glDrawArrays(GL_POINTS, 0, point_vertex_count);
        glBindVertexArray(0);
    }
}

