#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../include/shader.hpp"

#include "../include/utils.hpp"
#include "../include/half_edge.hpp"


// Globals for interaction
int visible_segments = 1;
bool space_was_pressed = false;
int selected_vertex = 0;
bool tab_was_pressed = false;


#include "../include/handle_input.hpp"

int main() {
    GLFWwindow* window = setupGLFW();
    if (!window) {
        return -1;
    }

    // Load OBJ points and faces
    std::string objPath = "assets/bunny.obj"; // or any OBJ file
    std::vector<Point> obj_points = loadOBJPoints(objPath);

    // Build half-edge mesh from OBJ data

    // Use OBJ points for editing/moving
    std::vector<Point> data_points = obj_points;

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

    visible_segments = data_points.size(); // Start by showing all segments

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Pass number of visible vertices (same as visible_segments)
        processInput(window, data_points.size(), visible_segments, data_points);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Recompute segments for current data_points (draw polygon outline dynamically)
        segments.clear();
        int n = data_points.size();
        for (int i = 0; i < n; ++i) {
            int j = (i + 1) % n;
            segments.push_back(drawSegmentByLineEquation(data_points[i].x, data_points[i].y, data_points[j].x, data_points[j].y));
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
