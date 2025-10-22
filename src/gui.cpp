#include "gui.hpp"
#include "half_edge.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

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

void renderGui(GuiState& state, Mesh& mesh, float& pov) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Half-Edge Operations");
    const char* operations[] = { "Faces of Vertex", "Edges of Vertex", "Adjacent Faces of Face", "Adjacent Faces of Edge" };
    ImGui::Combo("Operation", &state.operation, operations, IM_ARRAYSIZE(operations));
    ImGui::SliderFloat("POV Angle", &pov, 10.f, 90.0f);

    if (state.operation == 0 && !mesh.verticesHE.empty()) {
        ImGui::SliderInt("Vertex Index", &state.selected_vertex, 0, (int)mesh.verticesHE.size() - 1);
        if (ImGui::Button("Run Query")) {
            state.results.clear();
            state.highlighted_vertices.clear();
            auto faces = getFacesOfVertex(&mesh.verticesHE[state.selected_vertex]);
            for (auto* f : faces) {
                state.results.push_back("Face index: " + std::to_string(f - &mesh.facesHE[0]));
                HalfEdge* start_he = f->edge;
                HalfEdge* current_he = start_he;
                do {
                    Vertex* v1 = current_he->origin;
                    Vertex* v2 = current_he->next->origin;
                    state.highlighted_vertices.insert(state.highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                    current_he = current_he->next;
                } while (current_he != start_he);
            }
        }
    } else if (state.operation == 1 && !mesh.verticesHE.empty()) {
        ImGui::SliderInt("Vertex Index", &state.selected_vertex, 0, (int)mesh.verticesHE.size() - 1);
        if (ImGui::Button("Run Query")) {
            state.results.clear();
            state.highlighted_vertices.clear();
            auto edges = getEdgesOfVertex(&mesh.verticesHE[state.selected_vertex]);
            for (auto* e : edges) {
                state.results.push_back("Edge index: " + std::to_string(e - &mesh.halfedgesHE[0]));
                Vertex* v1 = e->origin;
                Vertex* v2 = e->next->origin;
                state.highlighted_vertices.insert(state.highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
            }
        }
    } else if (state.operation == 2 && !mesh.facesHE.empty()) {
        ImGui::SliderInt("Face Index", &state.selected_face, 0, (int)mesh.facesHE.size() - 1);
        if (ImGui::Button("Run Query")) {
            state.results.clear();
            state.highlighted_vertices.clear();
            auto adj_faces = getAdjacentFacesOfFace(&mesh.facesHE[state.selected_face]);
            for (auto* f : adj_faces) {
                state.results.push_back("Adjacent Face: " + std::to_string(f - &mesh.facesHE[0]));
                HalfEdge* start_he = f->edge;
                HalfEdge* current_he = start_he;
                do {
                    Vertex* v1 = current_he->origin;
                    Vertex* v2 = current_he->next->origin;
                    state.highlighted_vertices.insert(state.highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                    current_he = current_he->next;
                } while (current_he != start_he);
            }
        }
    } else if (state.operation == 3 && !mesh.halfedgesHE.empty()) {
        ImGui::SliderInt("Edge Index", &state.selected_edge, 0, (int)mesh.halfedgesHE.size() - 1);
        if (ImGui::Button("Run Query")) {
            state.results.clear();
            state.highlighted_vertices.clear();
            auto adj_faces = getAdjacentFacesOfEdge(&mesh.halfedgesHE[state.selected_edge]);
            for(auto* f : adj_faces) {
                state.results.push_back("Adjacent Face: " + std::to_string(f - &mesh.facesHE[0]));
                HalfEdge* start_he = f->edge;
                HalfEdge* current_he = start_he;
                do {
                    Vertex* v1 = current_he->origin;
                    Vertex* v2 = current_he->next->origin;
                    state.highlighted_vertices.insert(state.highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                    current_he = current_he->next;
                } while (current_he != start_he);
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Results:");
    if (state.results.empty()) {
        ImGui::Text("None");
    } else {
        for (const auto& r : state.results) {
            ImGui::Text("%s", r.c_str());
        }
    }
    ImGui::Separator();
    ImGui::Text("Render Mode");
    if (ImGui::RadioButton("DDA Points", mesh.currentRenderMode == Mesh::RenderMode::POINTS)) {
        mesh.setRenderMode(Mesh::RenderMode::POINTS);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("GL_LINES", mesh.currentRenderMode == Mesh::RenderMode::LINES)) {
        mesh.setRenderMode(Mesh::RenderMode::LINES);
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("XIAOLIN_WU", mesh.currentRenderMode == Mesh::RenderMode::XIAOLIN_WU)) {
        mesh.setRenderMode(Mesh::RenderMode::XIAOLIN_WU);
    }
    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
