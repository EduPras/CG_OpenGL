/**
 * @file mesh.hpp
 * @brief Handles mesh loading, transformations, and rendering.
 */
#pragma once
#include <vector>
#include <string>
#include "utils.hpp"
#include "half_edge.hpp"
#include <glm/glm.hpp>
#include "shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct WuVertex {
    glm::vec2 position; // 2D screen position
    glm::vec4 color;    // Color with intensity in alpha
};

class Mesh {
    private:
        glm::mat4 modelMatrix = glm::mat4(1.0f); // Identity matrix
        Shader *shader;
        size_t wu_vbo_allocated_size = 0;

    public:
        // Enum to define the available rendering modes
        enum RenderMode {
            LINES,
            POINTS,
            XIAOLIN_WU,
            BRESENHAM
        };
    // For Bresenham rendering
    std::vector<WuVertex> bresenham_vertex_buffer;
    unsigned int bresenham_point_count = 0;

        // If BRESENHAM use this function
        void drawWithBresenham(
            Shader* shader,
            const glm::mat4& model,
            const glm::mat4& view,
            const glm::mat4& projection,
            int screenWidth,
            int screenHeight
        );
        // Raw data loaded from the OBJ file
        std::vector<Point> points;
        std::vector<std::vector<int>> face_indices;

        // Half-edge data structures
        std::vector<Vertex> verticesHE;
        std::vector<HalfEdge> halfedgesHE;
        std::vector<Face> facesHE;

        // For GL_LINES rendering
        unsigned int VAO_lines, VBO_lines, EBO_lines;
        unsigned int line_index_count;
        // For GL_POINTS rendering (using DDA)
        unsigned int VAO_points, VBO_points;
        unsigned int point_vertex_count;
        // For XIOLIN_WU rendering
        unsigned int VAO_wu, VBO_wu;
        std::vector<std::pair<unsigned int, unsigned int>> edge_indices;
        std::vector<WuVertex> wu_vertex_buffer;
        unsigned int wu_point_count = 0;

        // The currently active rendering mode
        RenderMode currentRenderMode;

        Mesh();
        bool loadFromOBJ(const std::string& filename);
        void buildHalfEdge();
        

        void translate(const glm::vec3& trans);
        void rotate(float angle, const glm::vec3& axis);
        void scale(const glm::vec3& scaleVec);
        void resetTransformations();

        void setShader(Shader* shader) {
            this->shader = shader;
            this->shader->activate();
        }
        void setupMesh(); 
        // If GL_LINES or DDA use this function
        void draw(glm::mat4& view, glm::mat4& projection, Shader *shader, int screenWidth, int screenHeight);
        // Use this function to change the rendering mode
        void setRenderMode(RenderMode newMode);
        // If XIAOLIN_WU use this function
        void drawWithXiaolinWu(
            Shader* shader, 
            const glm::mat4& model, 
            const glm::mat4& view, 
            const glm::mat4& projection, 
            int screenWidth, 
            int screenHeight
        );
};
