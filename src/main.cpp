#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../include/shader.hpp"
#include "../include/utils.hpp"


// Globals for interaction
int visible_segments = 1;
bool space_was_pressed = false;
int selected_vertex = 0;
bool tab_was_pressed = false;

void processInput(GLFWwindow *window, int total_segments, int visible_vertices, std::vector<Point>& data_points) {
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

    // TAB: select next visible vertex
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
        if (!tab_was_pressed) {
            selected_vertex = (selected_vertex + 1) % visible_vertices;
            tab_was_pressed = true;
        }
    } else {
        tab_was_pressed = false;
    }

    // Arrow keys: move selected vertex
    float move_step = 0.02f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        data_points[selected_vertex].y += move_step;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        data_points[selected_vertex].y -= move_step;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        data_points[selected_vertex].x -= move_step;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        data_points[selected_vertex].x += move_step;
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

    // Segments will be recomputed every frame as vertices may move
    std::vector<std::vector<Point>> segments;
    std::vector<float> vertices;

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
        // Pass number of visible vertices (same as visible_segments)
        processInput(window, data_points.size(), visible_segments, data_points);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Recompute segments for current data_points
        segments.clear();
        for (size_t i = 0; i < data_points.size(); ++i) {
            const Point& p1 = data_points[i];
            const Point& p2 = data_points[(i + 1) % data_points.size()];
            segments.push_back(drawSegmentByLineEquation(p1.x, p1.y, p2.x, p2.y));
        }

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

        // Highlight the selected vertex (draw as a larger point)
        glPointSize(10.0f);
        std::vector<float> sel = { data_points[selected_vertex].x, data_points[selected_vertex].y, data_points[selected_vertex].z };
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sel.size() * sizeof(float), sel.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, 1);
        glPointSize(1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
