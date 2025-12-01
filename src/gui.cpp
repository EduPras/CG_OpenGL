#include "gui.hpp"
#include "half_edge.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "input.hpp"

void setupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void shutdownImGui() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void renderGui(GuiState& state, Mesh& mesh, TransformState* transformState) {

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Show current mode (after NewFrame)
    if (isShearModeActive()) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0, 1), "SHEAR MODE ACTIVE");
    }
    if (isViewportMode()) {
        ImGui::TextColored(ImVec4(0.5f, 0.8f, 1, 1), "Viewport Transformation Mode");
    } else {
        ImGui::TextColored(ImVec4(0.2f, 1, 0.2f, 1), "Object Transformation Mode");
    }

    // Show selected object name
    if (!state.object_names.empty()) {
        state.selected_object_name = state.object_names[state.selected_object];
        ImGui::Text("Selected Object: %s", state.selected_object_name.c_str());
    }

    ImGui::SliderFloat("POV Angle", &transformState->pov, 10.f, 90.0f);
    if (ImGui::Button("Save state")) {
        saveTransformState("state.json", *transformState);
    }

    ImGui::Separator();
    ImGui::Text("Render Mode");

    ImGui::SameLine();
    if (ImGui::RadioButton("XIAOLIN_WU", mesh.currentRenderMode == Mesh::RenderMode::XIAOLIN_WU)) {
        mesh.setRenderMode(Mesh::RenderMode::XIAOLIN_WU);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
