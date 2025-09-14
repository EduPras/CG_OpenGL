#include "../include/handle_input.hpp"
#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Globals for interaction
extern int visible_segments;
extern bool space_was_pressed;
extern int selected_vertex;
extern bool tab_was_pressed;

void processInput(GLFWwindow *window, int total_segments, int visible_vertices, std::vector<Point>& data_points, float& zoom_level, float& rotation_angle, float& pan_x, float& pan_y) {
    // +/-: zoom in/out
    if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
        zoom_level *= 1.01f;
    }
    if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
        zoom_level /= 1.01f;
    }

    // Q/E: rotate left/right
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        rotation_angle -= 0.01f;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        rotation_angle += 0.01f;
    }
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

    // Arrow keys: pan viewport
    float pan_step = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        pan_y += pan_step;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        pan_y -= pan_step;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        pan_x -= pan_step;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        pan_x += pan_step;
    }
}
