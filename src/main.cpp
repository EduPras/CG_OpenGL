#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../include/shader.hpp"

#include "../include/utils.hpp"
#include "../include/mesh.hpp"
#include "../include/viewer.hpp"
#include "../include/input.hpp"


// Interaction state
int visible_segments = 1;
int selected_vertex = 0;
float zoom_level = 1.0f;
float rotation_angle = 0.0f;
float pan_x = 0.0f, pan_y = 0.0f;
bool dragging = false;
int rotate_axis = 0;
double last_mouse_x = 0.0, last_mouse_y = 0.0;

int main() {

    GLFWwindow* window = setupGLFW();

    if (!window) {
        return -1;
    }

    // --- Modularized version ---
    Mesh mesh;
    mesh.loadFromOBJ("assets/bunny.obj");
    mesh.buildHalfEdge();

    Viewer viewer;
    viewer.setupGL();
    Shader shader("shaders/vertex_core.glsl", "shaders/fragment_core.glsl");
    viewer.setShader(&shader);

    // Input state
    dragging = false;
    rotate_axis = 0;
    last_mouse_x = 0.0;
    last_mouse_y = 0.0;
    setupInputCallbacks(
        window, &mesh, &viewer,
        &rotate_axis, &dragging,
        &last_mouse_x, &last_mouse_y,
        &zoom_level,
        &pan_x, &pan_y
    );

    visible_segments = mesh.points.size();
    selected_vertex = 0;

    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Apply zoom and rotation to a copy of mesh.points for rendering
        std::vector<Point> transformed_points = mesh.points;
        float cos_theta = cos(rotation_angle);
        float sin_theta = sin(rotation_angle);
        for (auto& p : transformed_points) {
            float x_new = p.x * cos_theta - p.y * sin_theta;
            float y_new = p.x * sin_theta + p.y * cos_theta;
            p.x = x_new * zoom_level + pan_x;
            p.y = y_new * zoom_level + pan_y;
            p.z = p.z * zoom_level;
        }

        viewer.drawMeshPoints(mesh, transformed_points);

        // Highlight the selected vertex (draw as a larger point)
        glPointSize(10.0f);
        std::vector<float> sel = { transformed_points[selected_vertex].x, transformed_points[selected_vertex].y, transformed_points[selected_vertex].z };
        glBindBuffer(GL_ARRAY_BUFFER, viewer.VBO);
        glBufferData(GL_ARRAY_BUFFER, sel.size() * sizeof(float), sel.data(), GL_DYNAMIC_DRAW);
        glDrawArrays(GL_POINTS, 0, 1);
        glPointSize(1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
