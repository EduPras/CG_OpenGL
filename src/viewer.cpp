#include "../include/viewer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>


// Initialize OpenGL VAO and VBO for point rendering
void Viewer::setupGL() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Set up vertex attribute for 3D points
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// Set the shader to use for rendering
void Viewer::setShader(Shader* s) {
    shader = s;
}

// Draw the mesh as points, using the provided transformed points
void Viewer::drawMeshPoints(const Mesh& mesh, const std::vector<Point>& transformed_points) {
    std::vector<float> vertices;
    // For each face, draw its edges as a sequence of points
    for (const auto& face : mesh.faces) {
        int m = face.size();
        for (int i = 0; i < m; ++i) {
            int idx0 = face[i];
            int idx1 = face[(i+1)%m];
            const auto& p0 = transformed_points[idx0];
            const auto& p1 = transformed_points[idx1];
            // Use DDA to get points along the edge
            std::vector<Point> seg_points = drawSegmentByLineEquation(p0.x, p0.y, p1.x, p1.y);
            for (const auto& pt : seg_points) {
                vertices.push_back(pt.x);
                vertices.push_back(pt.y);
                vertices.push_back(pt.z);
            }
        }
    }
    // Upload all edge points to the GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    // Activate shader and draw all points
    if (shader) shader->activate();
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
}
