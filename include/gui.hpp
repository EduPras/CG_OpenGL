#pragma once
#include <vector>
#include <string>
#include "mesh.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

// Holds all state needed for ImGui rendering and queries
struct GuiState {
    int operation = 0;
    int selected_vertex = 0;
    int selected_face = 0;
    int selected_edge = 0;
    std::vector<std::string> results;
    std::vector<float> highlighted_vertices;
};

void setupImGui(GLFWwindow* window);
void shutdownImGui();
void renderGui(GuiState& state, Mesh& mesh, float& pov);
