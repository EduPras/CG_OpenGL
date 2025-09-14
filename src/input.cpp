#include "../include/input.hpp"
#include <iostream>


// Global pointers to input state, set up by setupInputCallbacks
static Mesh* g_mesh = nullptr;
static int* g_rotate_axis = nullptr;
static bool* g_dragging = nullptr;
static double* g_last_mouse_x = nullptr;
static double* g_last_mouse_y = nullptr;
static float* g_zoom_level = nullptr;
static float* g_pan_x = nullptr;
static float* g_pan_y = nullptr;
// Mouse scroll callback: zoom in/out by changing zoom_level
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (g_zoom_level) {
        if (yoffset > 0) *g_zoom_level *= 1.05f; // Zoom in
        if (yoffset < 0) *g_zoom_level /= 1.05f; // Zoom out
    }
}

// Mouse button callback: start/stop dragging on left mouse button
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            *g_dragging = true;
            // Store the mouse position at the start of drag
            glfwGetCursorPos(window, g_last_mouse_x, g_last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            *g_dragging = false;
        }
    }
}

// Mouse move callback: handle dragging for pan (with CTRL) or rotation (with axis)
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (*g_dragging) {
        // Check if CTRL is held for panning
        int ctrl = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
        if (ctrl && g_pan_x && g_pan_y) {
            // Pan the viewport by updating pan_x and pan_y
            float dx = static_cast<float>(xpos - *g_last_mouse_x);
            float dy = static_cast<float>(ypos - *g_last_mouse_y);
            float pan_sensitivity = 0.005f;
            *g_pan_x += dx * pan_sensitivity;
            *g_pan_y -= dy * pan_sensitivity; // invert y for natural feel
            // Update last mouse position
            *g_last_mouse_x = xpos;
            *g_last_mouse_y = ypos;
        } else if (g_mesh && *g_rotate_axis != 0) {
            // Rotate the mesh around the selected axis
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
            // Update last mouse position
            *g_last_mouse_x = xpos;
            *g_last_mouse_y = ypos;
        }
    }
}

// Keyboard callback: select rotation axis (1=X, 2=Y, 3=Z), or clear on release
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

// Set up all input callbacks and store pointers to state variables
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
    // Store pointers to state so callbacks can update them
    g_mesh = mesh;
    g_rotate_axis = rotate_axis;
    g_dragging = dragging;
    g_last_mouse_x = last_mouse_x;
    g_last_mouse_y = last_mouse_y;
    g_zoom_level = zoom_level;
    g_pan_x = pan_x;
    g_pan_y = pan_y;
    // Register GLFW callbacks
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
}
