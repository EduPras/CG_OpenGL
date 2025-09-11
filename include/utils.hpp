#pragma once
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

struct Point {
    float x, y, z;
};

std::vector<Point> loadOBJPoints(const std::string& filename);
std::vector<Point> drawSegmentByLineEquation(float x1, float y1, float x2, float y2);
GLFWwindow* setupGLFW();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
