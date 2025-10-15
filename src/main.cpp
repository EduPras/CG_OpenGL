#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.hpp"
#include "mesh.hpp"
#include "input.hpp"
#include "half_edge.hpp"
#include "utils.hpp"

// --- Global variables for camera and input state ---
float zoom_level = 1.0f;
float rotation_angle_y = 0.0f;
float rotation_angle_x = 0.0f;
int WIDTH = 1080, HEIGHT = 1080;
glm::vec2 pan_offset = glm::vec2(0.0f, 0.0f);


int main(int argc, char* argv[]) {
    // -- Load file --
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1; 
    }
    std::string filename = argv[1]; 

    // --- Setup OpenGL and ImGui ---
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
    setupInputCallbacks(window, &zoom_level, &rotation_angle_x, &rotation_angle_y, &pan_offset);
    
    // --- Setup for Highlight Rendering ---
    GLuint highlightVAO, highlightVBO;
    glGenVertexArrays(1, &highlightVAO);
    glGenBuffers(1, &highlightVBO);
    glBindVertexArray(highlightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    std::vector<float> highlighted_vertices;

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.5f); 
    glEnable(GL_LINE_SMOOTH);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Get MVP matrices
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        float aspect_ratio = (height > 0) ? (float)width / (float)height : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(30.0f), aspect_ratio, 0.1f, 100.0f);
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(pan_offset.x, pan_offset.y, -3.0f / zoom_level));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, rotation_angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, rotation_angle_x, glm::vec3(1.0f, 0.0f, 0.0f));

        // Draw the mesh
        // --- 1. Draw the main mesh in white ---
        if (mesh.currentRenderMode != Mesh::RenderMode::NONE){
        // Defining shaders
            gpu_shader.setMat4("u_model", model);
            gpu_shader.setMat4("u_view", view);
            gpu_shader.setMat4("u_projection", projection);
            gpu_shader.setVec4("ourColor", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        
            mesh.draw(view, projection, &gpu_shader, width, height);
        }
        else{
            mesh.drawWithXiaolinWu(&wu_shader, model, view, projection, width, height);
        }

        // --- 2. Draw the highlighted geometry in green ---
        if (!highlighted_vertices.empty()) {
            // *** Disable depth testing to ensure highlights are visible on top ***
            glDisable(GL_DEPTH_TEST);

            gpu_shader.setVec4("ourColor", glm::vec4(0.1f, 1.0f, 0.2f, 1.0f));
            
            glBindVertexArray(highlightVAO);
            glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
            glBufferData(GL_ARRAY_BUFFER, highlighted_vertices.size() * sizeof(float), highlighted_vertices.data(), GL_DYNAMIC_DRAW);
            
            glDrawArrays(GL_LINES, 0, highlighted_vertices.size() / 3);
            glBindVertexArray(0);
            // *** Re-enable depth testing for the rest of the scene ***
            glEnable(GL_DEPTH_TEST);
        }

        // --- ImGui Rendering ---
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Half-Edge Operations");
        static int operation = 0;
        const char* operations[] = { "Faces of Vertex", "Edges of Vertex", "Adjacent Faces of Face", "Adjacent Faces of Edge" };
        ImGui::Combo("Operation", &operation, operations, IM_ARRAYSIZE(operations));

        static int selected_vertex = 0;
        static int selected_face = 0;
        static int selected_edge = 0;
        static std::vector<std::string> results;

        // --- IMGUI LOGIC WITH COMPLETE HIGHLIGHTING ---
        if (operation == 0 && !mesh.verticesHE.empty()) { // Faces of Vertex
            ImGui::SliderInt("Vertex Index", &selected_vertex, 0, (int)mesh.verticesHE.size() - 1);
            if (ImGui::Button("Run Query")) {
                results.clear();
                highlighted_vertices.clear();
                auto faces = getFacesOfVertex(&mesh.verticesHE[selected_vertex]);
                for (auto* f : faces) {
                    results.push_back("Face index: " + std::to_string(f - &mesh.facesHE[0]));
                    HalfEdge* start_he = f->edge;
                    HalfEdge* current_he = start_he;
                    do {
                        Vertex* v1 = current_he->origin;
                        Vertex* v2 = current_he->next->origin;
                        highlighted_vertices.insert(highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                        current_he = current_he->next;
                    } while (current_he != start_he);
                }
            }
        } else if (operation == 1 && !mesh.verticesHE.empty()) { // Edges of Vertex
            ImGui::SliderInt("Vertex Index", &selected_vertex, 0, (int)mesh.verticesHE.size() - 1);
            if (ImGui::Button("Run Query")) {
                results.clear();
                highlighted_vertices.clear();
                auto edges = getEdgesOfVertex(&mesh.verticesHE[selected_vertex]);
                for (auto* e : edges) {
                    results.push_back("Edge index: " + std::to_string(e - &mesh.halfedgesHE[0]));
                    Vertex* v1 = e->origin;
                    Vertex* v2 = e->next->origin;
                    highlighted_vertices.insert(highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                }
            }
        } else if (operation == 2 && !mesh.facesHE.empty()) { // Adjacent Faces of Face
             ImGui::SliderInt("Face Index", &selected_face, 0, (int)mesh.facesHE.size() - 1);
             if (ImGui::Button("Run Query")) {
                results.clear();
                highlighted_vertices.clear();
                auto adj_faces = getAdjacentFacesOfFace(&mesh.facesHE[selected_face]);
                for (auto* f : adj_faces) {
                    results.push_back("Adjacent Face: " + std::to_string(f - &mesh.facesHE[0]));
                    HalfEdge* start_he = f->edge;
                    HalfEdge* current_he = start_he;
                    do {
                        Vertex* v1 = current_he->origin;
                        Vertex* v2 = current_he->next->origin;
                        highlighted_vertices.insert(highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                        current_he = current_he->next;
                    } while (current_he != start_he);
                }
            }
        } else if (operation == 3 && !mesh.halfedgesHE.empty()) { // Adjacent Faces of Edge
            ImGui::SliderInt("Edge Index", &selected_edge, 0, (int)mesh.halfedgesHE.size() - 1);
            if (ImGui::Button("Run Query")) {
                results.clear();
                highlighted_vertices.clear();
                auto adj_faces = getAdjacentFacesOfEdge(&mesh.halfedgesHE[selected_edge]);
                for(auto* f : adj_faces) {
                    results.push_back("Adjacent Face: " + std::to_string(f - &mesh.facesHE[0]));
                    HalfEdge* start_he = f->edge;
                    HalfEdge* current_he = start_he;
                    do {
                        Vertex* v1 = current_he->origin;
                        Vertex* v2 = current_he->next->origin;
                        highlighted_vertices.insert(highlighted_vertices.end(), {v1->x, v1->y, v1->z, v2->x, v2->y, v2->z});
                        current_he = current_he->next;
                    } while (current_he != start_he);
                }
            }
        }

        ImGui::Separator();
        ImGui::Text("Results:");
        if (results.empty()) {
            ImGui::Text("None");
        } else {
            for (const auto& r : results) {
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
        if (ImGui::RadioButton("NONE", mesh.currentRenderMode == Mesh::RenderMode::NONE)) {
            mesh.setRenderMode(Mesh::RenderMode::NONE);
        }

        ImGui::End();
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    glDeleteVertexArrays(1, &highlightVAO);
    glDeleteBuffers(1, &highlightVBO);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

