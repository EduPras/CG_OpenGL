#pragma once
#include <GLFW/glfw3.h>
#include "mesh.hpp"
#include "viewer.hpp"


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

