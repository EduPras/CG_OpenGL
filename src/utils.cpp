
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>    
#include <algorithm> 
#include <glad/glad.h> // keep this
#include <GLFW/glfw3.h> // keep this
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "utils.hpp"


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
std::vector<Point> drawSegmentByLineEquation3D(const Point& p1, const Point& p2) {
    std::vector<Point> points;
    
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    float dz = p2.z - p1.z;

    // Find the dimension with the greatest change to determine the number of steps
    float max_delta = std::max(std::max(std::abs(dx), std::abs(dy)), std::abs(dz));
    
    // Increase density for a more solid line look. Adjust this multiplier as needed.
    int steps = static_cast<int>(max_delta * 250);

    if (steps == 0) {
        points.push_back(p1);
        return points;
    }

    float x_inc = dx / steps;
    float y_inc = dy / steps;
    float z_inc = dz / steps;

    float x = p1.x;
    float y = p1.y;
    float z = p1.z;

    for (int i = 0; i <= steps; ++i) {
        points.push_back({x, y, z});
        x += x_inc;
        y += y_inc;
        z += z_inc;
    }
    return points;
}

// GLFW framebuffer resize callback: update OpenGL viewport
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

glm::vec2 projectWorldToScreen(
    glm::vec3 worldPos, const glm::mat4& view, const glm::mat4& projection, int windowWidth, int windowHeight) {
    // Combine matrices and transform the 3D point to Clip Space
    glm::vec4 clipCoords = projection * view * glm::vec4(worldPos, 1.0f);
    // Perform perspective divide to get Normalized Device Coordinates (NDC)
    glm::vec3 ndcCoords = glm::vec3(clipCoords) / clipCoords.w;
    // Check if the point is behind the camera (clipped)
    if (clipCoords.w < 0) {
        return glm::vec2(NAN, NAN); // Or some other invalid value
    }
    // Perform viewport transform to get Screen Coordinates
    float screenX = (ndcCoords.x + 1.0f) / 2.0f * windowWidth;
    float screenY = (1.0f - ndcCoords.y) / 2.0f * windowHeight;
    return glm::vec2(screenX, screenY);
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