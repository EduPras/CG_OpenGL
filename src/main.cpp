#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include "../include/shader.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../include/utils.hpp"
#include "../include/mesh.hpp"
#include "../include/viewer.hpp"
#include "../include/input.hpp"



/**
 * @brief Number of visible segments in the mesh.
 */
int visible_segments = 1;

/**
 * @brief Index of the currently selected vertex.
 */
int selected_vertex = 0;

/**
 * @brief Current zoom level for the viewport.
 */
float zoom_level = 1.0f;

/**
 * @brief Current rotation angle (radians) for the viewport.
 */
float rotation_angle = 0.0f;

/**
 * @brief Current pan offset for the viewport (x, y).
 */
float pan_x = 0.0f, pan_y = 0.0f;

/**
 * @brief Whether the mouse is currently dragging.
 */
bool dragging = false;

/**
 * @brief Current axis of rotation (0: none, 1: X, 2: Y, 3: Z).
 */
int rotate_axis = 0;

/**
 * @brief Last mouse position (x, y).
 */
double last_mouse_x = 0.0, last_mouse_y = 0.0;


/**
 * @brief Main entry point. Initializes modules and runs the main loop.
 *
 * Sets up the OpenGL context, loads the mesh, initializes the viewer and input callbacks,
 * and enters the main rendering loop. Handles viewport transformations and drawing.
 *
 * @return Exit code.
 */
int main() {

    GLFWwindow* window = setupGLFW();

    if (!window) {
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // --- Modularized version ---
    Mesh mesh;
    mesh.loadFromOBJ("assets/cube.obj");
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

        // glfwSwapBuffers(window);
        // glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Your ImGui UI code here
        ImGui::Begin("Mesh Info");
        ImGui::Text("Vertices: %zu", mesh.points.size());
        ImGui::Text("Edges: %zu", mesh.halfedgesHE.size());
        ImGui::Text("Faces: %zu", mesh.faces.size());
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
