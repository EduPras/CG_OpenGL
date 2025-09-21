#include "input.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// --- Global State for Input Handling ---
// We store pointers to the variables in main.cpp that we need to modify.
struct InputState {
    float* zoom_level;
    float* rotation_angle_x;
    float* rotation_angle_y;
    glm::vec2* pan_offset;

    bool is_dragging_left = false;  // For rotation
    bool is_dragging_right = false; // For panning
    double last_mouse_x = 0.0;
    double last_mouse_y = 0.0;
};

// A single global instance of our input state
static InputState input_state;


// --- Callback Functions ---

// Called when a mouse button is pressed or released
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // We update the dragging state based on the left and right mouse buttons
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            input_state.is_dragging_left = true;
            glfwGetCursorPos(window, &input_state.last_mouse_x, &input_state.last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            input_state.is_dragging_left = false;
        }
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            input_state.is_dragging_right = true;
            glfwGetCursorPos(window, &input_state.last_mouse_x, &input_state.last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            input_state.is_dragging_right = false;
        }
    }
}

// Called when the mouse cursor moves
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!input_state.is_dragging_left && !input_state.is_dragging_right) {
        return; // Do nothing if we're not dragging
    }

    // Calculate the change in mouse position from the last frame
    double dx = xpos - input_state.last_mouse_x;
    double dy = ypos - input_state.last_mouse_y;

    if (input_state.is_dragging_left) {
        // Left-click drag rotates the model.
        // The sensitivity can be adjusted by changing the multiplier.
        *input_state.rotation_angle_y += static_cast<float>(dx) * 0.005f;
        *input_state.rotation_angle_x += static_cast<float>(dy) * 0.005f;
    }

    if (input_state.is_dragging_right) {
        // Right-click drag pans the model.
        // We need to find the window dimensions to scale the panning correctly.
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        if (width > 0 && height > 0) {
            input_state.pan_offset->x += static_cast<float>(dx) / static_cast<float>(width) * 2.0f;
            input_state.pan_offset->y -= static_cast<float>(dy) / static_cast<float>(height) * 2.0f; // Y is inverted
        }
    }

    // Update the last mouse position for the next frame
    input_state.last_mouse_x = xpos;
    input_state.last_mouse_y = ypos;
}

// Called when the mouse wheel is scrolled
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    // yoffset will be +1 for scrolling up (zoom in) and -1 for scrolling down (zoom out)
    // We adjust the zoom level, ensuring it doesn't go below a small positive value.
    float zoom_change = *input_state.zoom_level * 0.1f * static_cast<float>(yoffset);
    *input_state.zoom_level += zoom_change;

    if (*input_state.zoom_level < 0.1f) {
        *input_state.zoom_level = 0.1f;
    }
}

// Called when a key is pressed
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}


// --- Main Setup Function ---

void setupInputCallbacks(GLFWwindow* window, float* zoom, float* rot_x, float* rot_y, glm::vec2* pan) {
    // Store the pointers to the variables in main.cpp
    input_state.zoom_level = zoom;
    input_state.rotation_angle_x = rot_x;
    input_state.rotation_angle_y = rot_y;
    input_state.pan_offset = pan;

    // Register all our callback functions with GLFW
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}