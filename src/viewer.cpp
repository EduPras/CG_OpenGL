#include "../include/viewer.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

void Viewer::setupGL() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void Viewer::setShader(Shader* s) {
    shader = s;
}

void Viewer::drawMeshPoints(const Mesh& mesh, const std::vector<Point>& transformed_points) {
    std::vector<float> vertices;
    for (const auto& face : mesh.faces) {
        int m = face.size();
        for (int i = 0; i < m; ++i) {
            int idx0 = face[i];
            int idx1 = face[(i+1)%m];
            const auto& p0 = transformed_points[idx0];
            const auto& p1 = transformed_points[idx1];
            std::vector<Point> seg_points = drawSegmentByLineEquation(p0.x, p0.y, p1.x, p1.y);
            for (const auto& pt : seg_points) {
                vertices.push_back(pt.x);
                vertices.push_back(pt.y);
                vertices.push_back(pt.z);
            }
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    if (shader) shader->activate();
    glBindVertexArray(VAO);
    glDrawArrays(GL_POINTS, 0, vertices.size() / 3);
}
