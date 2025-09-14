#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

// Globals for viewport transformation
float zoom_level = 1.0f; // 1.0 = no zoom
float rotation_angle = 0.0f; // in radians
float pan_x = 0.0f, pan_y = 0.0f; // viewport translation

// Mouse drag state for rotating data points
static bool dragging = false;
static int rotate_axis = 0; // 1=X, 2=Y, 3=Z, 0=none
static double last_mouse_x = 0.0, last_mouse_y = 0.0;
static std::vector<Point>* g_data_points = nullptr;


#include "../include/handle_input.hpp"

int main() {

    GLFWwindow* window = setupGLFW();

    if (!window) {
        return -1;
    }

    // Load OBJ points and faces
    std::string objPath = "assets/bunny.obj"; // or any OBJ file
    std::vector<Point> obj_points = loadOBJPoints(objPath);
    std::vector<std::vector<int>> obj_faces = loadOBJFaces(objPath);

    // Use OBJ points for editing/moving
    std::vector<Point> data_points = obj_points;

    // Build half-edge mesh from OBJ data
    std::vector<Vertex> verticesHE;
    std::vector<HalfEdge> halfedgesHE;
    std::vector<Face> facesHE;
    buildHalfEdgeMeshFromPointsAndFaces(data_points, obj_faces, verticesHE, halfedgesHE, facesHE);

    // Segments will be recomputed every frame as vertices may move
    std::vector<std::vector<Point>> segments;
    std::vector<float> vertices;
    
    // Mouse callback for rotating all data points (no pan/zoom)
    g_data_points = &data_points;
    auto mouse_button_callback = [](GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS && rotate_axis != 0) {
                dragging = true;
                glfwGetCursorPos(window, &last_mouse_x, &last_mouse_y);
            } else if (action == GLFW_RELEASE) {
                dragging = false;
            }
        }
    };
    auto cursor_pos_callback = [](GLFWwindow* window, double xpos, double ypos) {
        if (dragging && g_data_points && rotate_axis != 0) {
            double delta = xpos - last_mouse_x;
            if (rotate_axis == 1) delta = ypos - last_mouse_y; // X axis: use vertical drag
            float sensitivity = 0.01f;
            float angle = sensitivity * static_cast<float>(delta);
            float cos_a = cos(angle);
            float sin_a = sin(angle);
            for (auto& p : *g_data_points) {
                if (rotate_axis == 1) { // X axis
                    float y_new = p.y * cos_a - p.z * sin_a;
                    float z_new = p.y * sin_a + p.z * cos_a;
                    p.y = y_new;
                    p.z = z_new;
                } else if (rotate_axis == 2) { // Y axis
                    float x_new = p.x * cos_a + p.z * sin_a;
                    float z_new = -p.x * sin_a + p.z * cos_a;
                    p.x = x_new;
                    p.z = z_new;
                } else if (rotate_axis == 3) { // Z axis
                    float x_new = p.x * cos_a - p.y * sin_a;
                    float y_new = p.x * sin_a + p.y * cos_a;
                    p.x = x_new;
                    p.y = y_new;
                }
            }
            last_mouse_x = xpos;
            last_mouse_y = ypos;
        }
    };
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    // Keyboard callback for axis selection
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_1) rotate_axis = 1;
            else if (key == GLFW_KEY_2) rotate_axis = 2;
            else if (key == GLFW_KEY_3) rotate_axis = 3;
        } else if (action == GLFW_RELEASE) {
            if (key == GLFW_KEY_1 && rotate_axis == 1) rotate_axis = 0;
            else if (key == GLFW_KEY_2 && rotate_axis == 2) rotate_axis = 0;
            else if (key == GLFW_KEY_3 && rotate_axis == 3) rotate_axis = 0;
        }
    });
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
        processInput(window, data_points.size(), visible_segments, data_points, zoom_level, rotation_angle, pan_x, pan_y);
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);


        // Apply zoom and rotation to a copy of data_points for rendering
        std::vector<Point> transformed_points = data_points;
        float cos_theta = cos(rotation_angle);
        float sin_theta = sin(rotation_angle);
        for (auto& p : transformed_points) {
            // Rotate around Z axis (2D)
            float x_new = p.x * cos_theta - p.y * sin_theta;
            float y_new = p.x * sin_theta + p.y * cos_theta;
            p.x = x_new * zoom_level + pan_x;
            p.y = y_new * zoom_level + pan_y;
            p.z = p.z * zoom_level; // Optional: scale z as well
        }

        // Draw mesh edges using face connectivity and drawSegmentByLineEquation
        vertices.clear();
        for (const auto& face : obj_faces) {
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

        shader.activate();
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, vertices.size() / 3);

        // Highlight the selected vertex (draw as a larger point)
        glPointSize(10.0f);
        std::vector<float> sel = { transformed_points[selected_vertex].x, transformed_points[selected_vertex].y, transformed_points[selected_vertex].z };
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
