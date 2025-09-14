#pragma once
#include <vector>
#include "utils.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class Viewer {
public:
    float zoom_level = 1.0f;
    float rotation_angle = 0.0f;
    float pan_x = 0.0f, pan_y = 0.0f;
    unsigned int VAO = 0, VBO = 0;
    Shader* shader = nullptr;

    void setupGL();
    void setShader(Shader* s);
    void drawMeshPoints(const Mesh& mesh, const std::vector<Point>& transformed_points);
};
