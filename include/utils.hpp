#pragma once
#include <vector>
#include <string>


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
std::vector<Point> drawSegmentByLineEquation(float x1, float y1, float x2, float y2);

/**
 * @brief Loads face indices from an OBJ file.
 * @param filename Path to the OBJ file.
 * @return Vector of faces, each as a vector of indices.
 */
std::vector<std::vector<int>> loadOBJFaces(const std::string& filename);

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
