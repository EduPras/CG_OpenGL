#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "imgui.h"

#include "input.hpp"

// We store pointers to the variables in main.cpp that we need to modify.

enum class RotationAxis {
    X,
    Y,
    Z
};


struct InputState {
    float* zoom_level;
    float* rotation_angle_x;
    float* rotation_angle_y;
    float* rotation_angle_z;
    glm::vec2* pan_offset;

    bool is_dragging_left = false;  // For both rotation and panning
    double last_mouse_x = 0.0;
    double last_mouse_y = 0.0;

    RotationAxis rotation_axis = RotationAxis::Y;

    // Shear mode state
    bool shear_mode = false;
    float shear_value = 0.0f;
};

// A single global instance of our input state
static InputState input_state;

// Shear state API
bool isShearModeActive() {
    return input_state.shear_mode;
}

float getShearValue() {
    return input_state.shear_value;
}

void resetShear() {
    input_state.shear_value = 0.0f;
}

void setShearValue(float value) {
    input_state.shear_value = value;
}


void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    // We only care about the left mouse button now
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            input_state.is_dragging_left = true;
            // Capture the initial mouse position when dragging starts
            glfwGetCursorPos(window, &input_state.last_mouse_x, &input_state.last_mouse_y);
        } else if (action == GLFW_RELEASE) {
            input_state.is_dragging_left = false;
        }
    }
}

// Called when the mouse cursor moves
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    if (!input_state.is_dragging_left) {
        return; 
    }

    // Calculate the change in mouse position from the last frame
    double dx = xpos - input_state.last_mouse_x;
    double dy = ypos - input_state.last_mouse_y;

    // We check the key state directly to allow pressing/releasing Shift during a drag.
    bool shift_pressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                          glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);


    if (shift_pressed) {
        // If Shift is down, we pan the model.
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        if (width > 0 && height > 0) {
            // Scale pan speed by zoom level to feel more natural
            float pan_speed = 2.0f / *input_state.zoom_level;
            input_state.pan_offset->x += static_cast<float>(dx) / static_cast<float>(width) * pan_speed;
            input_state.pan_offset->y -= static_cast<float>(dy) / static_cast<float>(height) * pan_speed; // Y is inverted
        }
    } else {
        // If Shift is NOT down, we rotate the model on the selected axis.
        float sensitivity = 0.005f;
        switch (input_state.rotation_axis) {
            case RotationAxis::X:
                *input_state.rotation_angle_x += static_cast<float>(dy) * sensitivity;
                break;
            case RotationAxis::Y:
                *input_state.rotation_angle_y += static_cast<float>(dx) * sensitivity;
                break;
            case RotationAxis::Z:
                *input_state.rotation_angle_z += static_cast<float>(dx) * sensitivity;
                break;
        }
    }

    // Update the last mouse position for the next frame
    input_state.last_mouse_x = xpos;
    input_state.last_mouse_y = ypos;
}

// Called when the mouse wheel is scrolled
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    // yoffset will be +1 for scrolling up (zoom in) and -1 for scrolling down (zoom out)
    float zoom_change = *input_state.zoom_level * 0.1f * static_cast<float>(yoffset);
    *input_state.zoom_level += zoom_change;

    // Clamp the zoom level to prevent it from becoming too small or inverted
    if (*input_state.zoom_level < 0.1f) {
        *input_state.zoom_level = 0.1f;
    }
}

// Called when a key is pressed

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_S) {
            input_state.shear_mode = !input_state.shear_mode;
            if (!input_state.shear_mode) {
                // Optionally reset shear value when exiting shear mode
                // input_state.shear_value = 0.0f;
            }
        } else if (input_state.shear_mode) {
            // Handle shearing with left/right arrows
            float shear_step = 0.05f;
            if (key == GLFW_KEY_LEFT) {
                input_state.shear_value -= shear_step;
            } else if (key == GLFW_KEY_RIGHT) {
                input_state.shear_value += shear_step;
            }
        } else {
            // Only allow rotation axis switching if not in shear mode
            if (key == GLFW_KEY_X) {
                input_state.rotation_axis = RotationAxis::X;
            } else if (key == GLFW_KEY_Y) {
                input_state.rotation_axis = RotationAxis::Y;
            } else if (key == GLFW_KEY_R) {
                input_state.rotation_axis = RotationAxis::Z;
            }
        }
    }
}

void setupInputCallbacks(GLFWwindow* window, float* zoom, float* rot_x, float* rot_y, float* rot_z, glm::vec2* pan ) {
    // Store the pointers to the variables in main.cpp
    input_state.zoom_level = zoom;
    input_state.rotation_angle_x = rot_x;
    input_state.rotation_angle_y = rot_y;
    input_state.rotation_angle_z = rot_z;
    input_state.pan_offset = pan;

    // Register all our callback functions with GLFW
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
}

