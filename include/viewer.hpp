#pragma once
#include <vector>
#include "utils.hpp"
#include "mesh.hpp"
#include "shader.hpp"

/**
 * @brief Handles OpenGL context, rendering, and viewport transformations.
 *
 * The Viewer class manages OpenGL buffers, shaders, and draws the mesh using transformed points.
 */
class Viewer {
public:
    /**
     * @brief Current zoom level for the viewport.
     */
    float zoom_level = 1.0f;

    /**
     * @brief Current rotation angle (radians) for the viewport.
     */
    float rotation_angle = 0.0f;

    /**
     * @brief Current pan offset for the viewport (x, y).
     */
    float pan_x = 0.0f, pan_y = 0.0f;

    /**
     * @brief OpenGL Vertex Array Object and Vertex Buffer Object.
     */
    unsigned int VAO = 0, VBO = 0;

    /**
     * @brief Pointer to the active shader program.
     */
    Shader* shader = nullptr;

    /**
     * @brief Initializes OpenGL buffers and attributes.
     */
    void setupGL();

    /**
     * @brief Sets the shader to use for rendering.
     * @param s Pointer to the Shader object.
     */
    void setShader(Shader* s);

    /**
     * @brief Draws the mesh using the provided transformed points.
     * @param mesh The mesh to draw.
     * @param transformed_points The points after applying transformations.
     */
    void drawMeshPoints(const Mesh& mesh, const std::vector<Point>& transformed_points);
};
