#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../include/shader.hpp"
#include "../include/utils.hpp"

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    GLFWwindow* window = setupGLFW();
    if (!window) {
        return -1;
    }

    // Choose which OBJ file to load:
    std::string objPath = "assets/nine_points.obj"; // Changed to nine_points.obj
    auto data_points = loadOBJPoints(objPath);
    if (data_points.size() < 2) {
        std::cerr << "OBJ file must contain at least two points." << std::endl;
        return 1;
    }

    // Use drawSegmentByLineEquation for each segment between consecutive points (including last to first)
    std::vector<Point> all_line_points;
    for (size_t i = 0; i < data_points.size(); ++i) {
        const Point& p1 = data_points[i];
        const Point& p2 = data_points[(i + 1) % data_points.size()]; // wrap around for closed loop
        auto segment = drawSegmentByLineEquation(p1.x, p1.y, p2.x, p2.y);
        all_line_points.insert(all_line_points.end(), segment.begin(), segment.end());
    }

    // Convert all generated line points to a flat float array for OpenGL
    std::vector<float> vertices;
    for (const auto& p : all_line_points) {
        vertices.push_back(p.x);
        vertices.push_back(p.y);
        vertices.push_back(p.z);
    }

    // Setup VAO and VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Load shaders
    Shader shader("shaders/vertex_core.glsl", "shaders/fragment_core.glsl");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.activate();
        glBindVertexArray(VAO);
        // Draw all generated points as GL_POINTS (line segments approximated by points)
        glDrawArrays(GL_POINTS, 0, all_line_points.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
