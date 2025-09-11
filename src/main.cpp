#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../include/shader.hpp"
#include "../include/utils.hpp"


// Global to track how many segments to show
int visible_segments = 1;
bool space_was_pressed = false;

void processInput(GLFWwindow *window, int total_segments) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // SPACE: reveal next segment
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!space_was_pressed) {
            visible_segments = std::min(visible_segments + 1, total_segments);
            space_was_pressed = true;
        }
    } else {
        space_was_pressed = false;
    }
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
    std::vector<std::vector<Point>> segments;
    for (size_t i = 0; i < data_points.size(); ++i) {
        const Point& p1 = data_points[i];
        const Point& p2 = data_points[(i + 1) % data_points.size()]; // wrap around for closed loop
        segments.push_back(drawSegmentByLineEquation(p1.x, p1.y, p2.x, p2.y));
    }

    // Flatten all points for all segments (initially just the first segment)
    std::vector<float> vertices;
    for (int seg = 0; seg < visible_segments; ++seg) {
        for (const auto& p : segments[seg]) {
            vertices.push_back(p.x);
            vertices.push_back(p.y);
            vertices.push_back(p.z);
        }
    }

    // Setup VAO and VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Load shaders
    Shader shader("shaders/vertex_core.glsl", "shaders/fragment_core.glsl");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window, segments.size());
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Update vertices for visible segments
        vertices.clear();
        for (int seg = 0; seg < visible_segments; ++seg) {
            for (const auto& p : segments[seg]) {
                vertices.push_back(p.x);
                vertices.push_back(p.y);
                vertices.push_back(p.z);
            }
        }
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

        shader.activate();
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
