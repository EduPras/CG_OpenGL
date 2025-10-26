/**
 * @file utils.hpp
 * @brief Utility functions and data structures.
 */
#pragma once
#include <glm/glm.hpp>
#include <string>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <string>
#include <glm/glm.hpp>

struct GLFWwindow;

/**
 * @brief 3D point or vertex.
 */
struct Point {
    float x, y, z;
};

/**
 * @brief Loads vertex positions from an OBJ file.
 * @param filename Path to the OBJ file.
 * @return Vector of Point structs.
 */
std::vector<Point> loadOBJPoints(const std::string& filename);

/**
 * @brief Generates points along a line segment using the DDA algorithm.
 * @param x1 Start x.
 * @param y1 Start y.
 * @param x2 End x.
 * @param y2 End y.
 * @return Vector of Point structs along the segment.
 */
std::vector<Point> drawSegmentByLineEquation3D(const Point& p1, const Point& p2);

/**
 * @brief Loads face indices from an OBJ file.
 * @param filename Path to the OBJ file.
 * @return Vector of faces, each as a vector of indices.
 */
std::vector<std::vector<int>> loadOBJFaces(const std::string& filename);

/**
 * @brief Projects a 3D point from world space to 2D screen space.
 * * @param worldPos The 3D point in world space.
 * @param view The camera's view matrix.
 * @param projection The camera's projection matrix.
 * @param windowWidth The width of the viewport/window in pixels.
 * @param windowHeight The height of the viewport/window in pixels.
 * @return The 2D screen coordinates of the projected point.
 */
// glm::vec2 projectWorldToScreen(glm::vec3 worldPos, const glm::mat4& view, const glm::mat4& projection, int windowWidth, int windowHeight);

/**
 * @brief Sets up GLFW and OpenGL context.
 * @return Pointer to the created GLFW window.
 */
GLFWwindow* setupGLFW();

/**
 * @brief GLFW framebuffer resize callback.
 * @param window Pointer to the GLFW window.
 * @param width New width.
 * @param height New height.
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

glm::vec2 projectWorldToScreen(
    glm::vec3 worldPos, 
    const glm::mat4& view, 
    const glm::mat4& projection, 
    int windowWidth, 
    int windowHeight);

struct TransformState {
    float zoom_level = 1.0f;
    float rotation_angle_x = 0.0f;
    float rotation_angle_y = 0.0f;
    float pov = 45.0f;
    glm::vec2 pan_offset = glm::vec2(0.0f, 0.0f);
};

// Save transformation state to a file (JSON format)
void saveTransformState(const std::string& filename, const TransformState& state);

// Load transformation state from a file (JSON format)
bool loadTransformState(const std::string& filename, TransformState& state);