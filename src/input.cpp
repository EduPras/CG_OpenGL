#include "../include/input.hpp"
#include <iostream>

static Mesh* g_mesh = nullptr;
static int* g_rotate_axis = nullptr;
static bool* g_dragging = nullptr;
static double* g_last_mouse_x = nullptr;
static double* g_last_mouse_y = nullptr;
static float* g_zoom_level = nullptr;
static float* g_pan_x = nullptr;
static float* g_pan_y = nullptr;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (g_zoom_level) {
        if (yoffset > 0) *g_zoom_level *= 1.05f;
        if (yoffset < 0) *g_zoom_level /= 1.05f;
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            *g_dragging = true;
            glfwGetCursorPos(window, g_last_mouse_x, g_last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            *g_dragging = false;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (*g_dragging) {
        int ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
        if (ctrl && g_pan_x && g_pan_y) {
            // Pan viewport
            float dx = static_cast<float>(xpos - *g_last_mouse_x);
            float dy = static_cast<float>(ypos - *g_last_mouse_y);
            float pan_sensitivity = 0.005f;
            *g_pan_x += dx * pan_sensitivity;
            *g_pan_y -= dy * pan_sensitivity; // invert y for natural feel
            *g_last_mouse_x = xpos;
            *g_last_mouse_y = ypos;
        } else if (g_mesh && *g_rotate_axis != 0) {
            double delta = xpos - *g_last_mouse_x;
            if (*g_rotate_axis == 1) delta = ypos - *g_last_mouse_y; // X axis: use vertical drag
            float sensitivity = 0.01f;
            float angle = sensitivity * static_cast<float>(delta);
            float cos_a = cos(angle);
            float sin_a = sin(angle);
            for (auto& p : g_mesh->points) {
                if (*g_rotate_axis == 1) { // X axis
                    float y_new = p.y * cos_a - p.z * sin_a;
                    float z_new = p.y * sin_a + p.z * cos_a;
                    p.y = y_new;
                    p.z = z_new;
                } else if (*g_rotate_axis == 2) { // Y axis
                    float x_new = p.x * cos_a + p.z * sin_a;
                    float z_new = -p.x * sin_a + p.z * cos_a;
                    p.x = x_new;
                    p.z = z_new;
                } else if (*g_rotate_axis == 3) { // Z axis
                    float x_new = p.x * cos_a - p.y * sin_a;
                    float y_new = p.x * sin_a + p.y * cos_a;
                    p.x = x_new;
                    p.y = y_new;
                }
            }
            *g_last_mouse_x = xpos;
            *g_last_mouse_y = ypos;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) *g_rotate_axis = 1;
        else if (key == GLFW_KEY_2) *g_rotate_axis = 2;
        else if (key == GLFW_KEY_3) *g_rotate_axis = 3;
    } else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_1 && *g_rotate_axis == 1) *g_rotate_axis = 0;
        else if (key == GLFW_KEY_2 && *g_rotate_axis == 2) *g_rotate_axis = 0;
        else if (key == GLFW_KEY_3 && *g_rotate_axis == 3) *g_rotate_axis = 0;
    }
}

void setupInputCallbacks(
    GLFWwindow* window,
    Mesh* mesh,
    Viewer* viewer,
    int* rotate_axis,
    bool* dragging,
    double* last_mouse_x,
    double* last_mouse_y,
    float* zoom_level,
    float* pan_x,
    float* pan_y
) {
    g_mesh = mesh;
    g_rotate_axis = rotate_axis;
    g_dragging = dragging;
    g_last_mouse_x = last_mouse_x;
    g_last_mouse_y = last_mouse_y;
    g_zoom_level = zoom_level;
    g_pan_x = pan_x;
    g_pan_y = pan_y;
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
}
