// Standard Libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

// Project Headers
#include "gui.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "input.hpp"
#include "half_edge.hpp"
#include "utils.hpp"

float zoom_level = 1.0f;
float rotation_angle_y = 0.0f;
float rotation_angle_x = 0.0f;
float pov = 45.0f;
int WIDTH = 1080, HEIGHT = 1080;
glm::vec2 pan_offset = glm::vec2(0.0f, 0.0f);


int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1; 
    }
    std::string filename = argv[1]; 

    GLFWwindow* window = setupGLFW();
    if (!window) return -1;

    Mesh mesh;
    if (!mesh.loadFromOBJ(filename)) {
        std::cerr << "Failed to load mesh, exiting." << std::endl;
        return -1;
    }
    mesh.buildHalfEdge();
    mesh.setupMesh();

    Shader gpu_shader("shaders/vertex_core.glsl", "shaders/fragment_core.glsl");
    Shader wu_shader("shaders/wu_line.vert", "shaders/wu_line.frag");

    wu_shader.setVec4("vertexColor", glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
    setupInputCallbacks(window, &zoom_level, &rotation_angle_x, &rotation_angle_y, &pan_offset);
    
    GLuint highlightVAO, highlightVBO;
    glGenVertexArrays(1, &highlightVAO);
    glGenBuffers(1, &highlightVBO);
    glBindVertexArray(highlightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    GuiState guiState;
    setupImGui(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get MVP matrices
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect_ratio = (height > 0) ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(pov), aspect_ratio, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(pan_offset.x, pan_offset.y, -3.0f / zoom_level));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, rotation_angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation_angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

        // Draw the mesh
        if (mesh.currentRenderMode != Mesh::RenderMode::XIAOLIN_WU) {
            gpu_shader.setMat4("u_model", model);
            gpu_shader.setMat4("u_view", view);
            gpu_shader.setMat4("u_projection", projection);
            gpu_shader.setVec4("ourColor", glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
            mesh.draw(view, projection, &gpu_shader, width, height);
        } else {
            wu_shader.setVec4("vertexColor", glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));
            mesh.drawWithXiaolinWu(&wu_shader, model, view, projection, width, height);
        }

        // Draw highlighted geometry in green
        if (!guiState.highlighted_vertices.empty()) {
            glDisable(GL_DEPTH_TEST);
            gpu_shader.setVec4("ourColor", glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
            glBindVertexArray(highlightVAO);
            glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
            glBufferData(GL_ARRAY_BUFFER, guiState.highlighted_vertices.size() * sizeof(float), guiState.highlighted_vertices.data(), GL_DYNAMIC_DRAW);
            glDrawArrays(GL_LINES, 0, guiState.highlighted_vertices.size() / 3);
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
        }

        renderGui(guiState, mesh, pov);
        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &highlightVAO);
    glDeleteBuffers(1, &highlightVBO);
    shutdownImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

