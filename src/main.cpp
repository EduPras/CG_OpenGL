// Standard Libraries
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
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

TransformState transformState;
int WIDTH = 1080, HEIGHT = 1080;

// Viewport rectangle (for clipping), defined as (x_min, y_min, x_max, y_max) in screen coordinates
// Default: 100px margin from each window edge
ViewportRect viewportRect = {100, 100, WIDTH - 100, HEIGHT - 100};


// Helper to load a mesh from file and add to vectors
void loadMeshObject(const std::string& filename, std::vector<Mesh>& objects, std::vector<std::string>& object_names) {
    Mesh mesh(filename);
    if (!mesh.loadFromOBJ(filename)) {
        std::cerr << "Failed to load mesh: " << filename << std::endl;
        return;
    }
    mesh.buildHalfEdge();
    mesh.setupMesh();
    mesh.setRenderMode(Mesh::XIAOLIN_WU);
    objects.push_back(mesh);
    object_names.push_back(filename);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename1> [filename2 ...]" << std::endl;
        return 1;
    }

    GLFWwindow* window = setupGLFW();
    if (!window) return -1;

    // Previous transform state
    loadTransformState("state.json", transformState);

    // Support multiple objects
    std::vector<Mesh> objects;
    std::vector<std::string> object_names;
    for (int i = 1; i < argc; ++i) {
        loadMeshObject(argv[i], objects, object_names);
    }
    if (objects.empty()) {
        std::cerr << "No valid meshes loaded. Exiting." << std::endl;
        return -1;
    }

    Shader wu_shader("shaders/wu_line.vert", "shaders/wu_line.frag");

    wu_shader.setVec4("vertexColor", glm::vec4(1.0f, 0.5f, 0.5f, 1.0f));

    GuiState guiState;
    guiState.selected_object = 0;
    guiState.object_names = object_names;
    setObjectTransformTargets(&objects, &guiState.selected_object);

    // Static callback for object selection
    static GuiState* guiStatePtr = nullptr;
    auto onObjectSelectFunc = [](int newIndex) {
        if (guiStatePtr) guiStatePtr->selected_object = newIndex;
    };
    guiStatePtr = &guiState;

    setObjectSelectCallback(onObjectSelectFunc, objects.size());
    setupInputCallbacks(window, &transformState.zoom_level, &transformState.rotation_angle_x, &transformState.rotation_angle_y, &transformState.rotation_angle_z,  &transformState.pan_offset);

    setupImGui(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update viewportRect in case window size changed
        int overlay_width, overlay_height;
        glfwGetFramebufferSize(window, &overlay_width, &overlay_height);

        // Get MVP matrices
        glm::mat4 model, view, projection;
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect_ratio = (height > 0) ? (float)width / (float)height : 1.0f;
        projection = glm::perspective(glm::radians(transformState.pov), aspect_ratio, 0.1f, 100.0f);
        view = glm::translate(glm::mat4(1.0f), glm::vec3(transformState.pan_offset.x, transformState.pan_offset.y, -3.0f / transformState.zoom_level));

        // Handle transformation mode
        for (size_t i = 0; i < objects.size(); ++i) {
            // Always start with viewport transform
            glm::mat4 model = glm::mat4(1.0f);
            // Shear: in viewport mode, apply to all; in object mode, do nothing here (handled in input)
            if (isShearModeActive() && isViewportMode()) {
                float sh = getShearValue();
                glm::mat4 shear = glm::mat4(1.0f);
                shear[1][0] = sh;
                model = shear * model;
            }
            model = glm::rotate(model, transformState.rotation_angle_z, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::rotate(model, transformState.rotation_angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, transformState.rotation_angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

            // Always apply per-object transform
            model = model * objects[i].objectTransform;

            glm::vec4 objColor = ((int)i == guiState.selected_object && !isViewportMode())
                ? glm::vec4(0.2f, 1.0f, 0.2f, 1.0f) // green
                : glm::vec4(1.0f, 0.5f, 0.5f, 1.0f); // default
            wu_shader.setVec4("vertexColor", objColor);
            objects[i].drawWithXiaolinWu(&wu_shader, model, view, projection, width, height, objColor, viewportRect);
        }

        // Pass selected object to GUI (for future selection logic)
        renderGui(guiState, objects[guiState.selected_object], &transformState, viewportRect);


        // Draw viewport rectangle overlay using ImGui
        ImDrawList* draw_list = ImGui::GetForegroundDrawList();
        ImVec2 p1 = ImVec2((float)viewportRect.x_min, (float)viewportRect.y_min);
        ImVec2 p2 = ImVec2((float)viewportRect.x_max, (float)viewportRect.y_min);
        ImVec2 p3 = ImVec2((float)viewportRect.x_max, (float)viewportRect.y_max);
        ImVec2 p4 = ImVec2((float)viewportRect.x_min, (float)viewportRect.y_max);
        draw_list->AddLine(p1, p2, IM_COL32(255, 255, 0, 255), 3.0f);
        draw_list->AddLine(p2, p3, IM_COL32(255, 255, 0, 255), 3.0f);
        draw_list->AddLine(p3, p4, IM_COL32(255, 255, 0, 255), 3.0f);
        draw_list->AddLine(p4, p1, IM_COL32(255, 255, 0, 255), 3.0f);

        glfwSwapBuffers(window);
    }

    // Cleanup
    shutdownImGui();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

