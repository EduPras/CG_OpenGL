/**
 * @file input.hpp
 * @brief Input and callbacks from inputs.
 */
#pragma once
#include <glm/glm.hpp>

// Forward-declare GLFWwindow to avoid including the full GLFW header here
struct GLFWwindow;

// Sets up all the necessary input callbacks for the GLFW window.
// It takes pointers to the main application's state variables that control the camera.

// Shear state accessors
bool isShearModeActive();
float getShearValue();
void resetShear();
void setShearValue(float value);

void setupInputCallbacks(
    GLFWwindow* window,
    float* zoom,
    float* rot_x,
    float* rot_y,
    float* rot_z,
    glm::vec2* pan
);
