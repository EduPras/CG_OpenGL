#pragma once
#include <GLFW/glfw3.h>
#include "mesh.hpp"
#include "viewer.hpp"
#include "imgui.h"

/**
 * @brief Sets up all input callbacks for interactive mesh viewing.
 *
 * Registers mouse, keyboard, and scroll callbacks to handle rotation, zoom, and panning.
 * All input state is managed via pointers to variables in main.
 *
 * @param window Pointer to the GLFW window.
 * @param mesh Pointer to the mesh object.
 * @param viewer Pointer to the viewer object.
 * @param rotate_axis Pointer to the rotation axis state variable.
 * @param dragging Pointer to the dragging state variable.
 * @param last_mouse_x Pointer to last mouse x position.
 * @param last_mouse_y Pointer to last mouse y position.
 * @param zoom_level Pointer to the zoom level variable.
 * @param pan_x Pointer to the pan x offset variable.
 * @param pan_y Pointer to the pan y offset variable.
 */
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
);


inline bool shouldBlockInput() {
    return ImGui::GetIO().WantCaptureMouse;
}
inline bool shouldBlockKeyboard() {
    return ImGui::GetIO().WantCaptureKeyboard;
}
