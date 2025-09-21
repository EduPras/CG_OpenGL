
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h> // keep this
#include <GLFW/glfw3.h> // keep this
#include "../include/utils.hpp"


// Load vertex positions from an OBJ file (lines starting with 'v ')
std::vector<Point> loadOBJPoints(const std::string& filename) {
    std::vector<Point> points;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 2) == "v ") {
            std::istringstream iss(line.substr(2));
            Point p;
            iss >> p.x >> p.y >> p.z;
            points.push_back(p);
        }
    }
    return points;
}


// Load face indices from an OBJ file (lines starting with 'f ')
std::vector<std::vector<int>> loadOBJFaces(const std::string& filename) {
    std::vector<std::vector<int>> faces;
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (line.size() > 0 && line[0] == 'f' && (line[1] == ' ' || line[1] == '\t')) {
            std::istringstream iss(line.substr(2));
            std::vector<int> inds;
            std::string vstr;
            while (iss >> vstr) {
                size_t slash = vstr.find('/');
                if (slash != std::string::npos) vstr = vstr.substr(0, slash);
                int idx = std::stoi(vstr);
                inds.push_back(idx - 1); // OBJ is 1-based
            }
            faces.push_back(inds);
        }
    }
    return faces;
}


// Generate points along a line segment using the DDA algorithm
std::vector<Point> drawSegmentByLineEquation(float x1, float y1, float x2, float y2) {
    std::vector<Point> points;
    // DDA algorithm: step along the axis with the largest difference
    float dx = x2 - x1;
    float dy = y2 - y1;
    int steps = static_cast<int>(std::max(std::abs(dx), std::abs(dy)) * 100); // 100 points per unit
    if (steps == 0) {
        points.push_back({x1, y1, 0.0f});
        return points;
    }
    float x_inc = dx / steps;
    float y_inc = dy / steps;
    float x = x1;
    float y = y1;
    for (int i = 0; i <= steps; ++i) {
        points.push_back({x, y, 0.0f});
        x += x_inc;
        y += y_inc;
    }
    return points;
}



// GLFW framebuffer resize callback: update OpenGL viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


// Set up GLFW and OpenGL context, return window pointer
GLFWwindow* setupGLFW(){
    // GLFW and OpenGL setup
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1080, 1080, "OBJ Line Viewer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        // return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        // return -1;
    }
    return window;
}