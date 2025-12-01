
/**
 * @file gui.hpp
 * @brief ImGui interface and state management for the mesh viewer application.
 */
#pragma once
#include <vector>
#include <string>
#include "mesh.hpp"
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

/**
 * @brief Holds all state needed for ImGui rendering and queries.
 */
struct GuiState {
    int operation = 0;              ///< Selected operation in the GUI combo box
    int selected_vertex = 0;        ///< Currently selected vertex index
    int selected_face = 0;          ///< Currently selected face index
    int selected_edge = 0;          ///< Currently selected edge index
    std::vector<std::string> results; ///< Query results to display in the GUI
    std::vector<float> highlighted_vertices; ///< Highlighted geometry for rendering

    // For object selection
    int selected_object = 0;
    std::vector<std::string> object_names;
    std::string selected_object_name;
};

/**
 * @brief Initializes ImGui for the given GLFW window.
 * @param window Pointer to the GLFW window.
 */
void setupImGui(GLFWwindow* window);

/**
 * @brief Shuts down and cleans up ImGui resources.
 */
void shutdownImGui();

/**
 * @brief Renders the ImGui interface and handles user interaction.
 * @param state Reference to the GUI state struct.
 * @param mesh Reference to the mesh object.
 * @param transformState Pointer to the current transformation state struct.
 */
void renderGui(GuiState& state, Mesh& mesh, TransformState* transformState);
