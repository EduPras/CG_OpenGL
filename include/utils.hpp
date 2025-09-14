// Parse face indices from OBJ file (returns vector of vector of indices)
#pragma once
#include <vector>
#include <string>

struct GLFWwindow;
struct Point {
    float x, y, z;
};

std::vector<Point> loadOBJPoints(const std::string& filename);
std::vector<Point> drawSegmentByLineEquation(float x1, float y1, float x2, float y2);
GLFWwindow* setupGLFW();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
