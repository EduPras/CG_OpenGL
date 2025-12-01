
#include <vector>
#include <utility>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <cmath> // Ensure cmath is included for std::abs

#include "utils.hpp"
#include "xiaolin_wu.hpp"
#include "weiler-atherton-clip.hpp"
#include "mesh.hpp"

// Constructor
Mesh::Mesh(const std::string& name_) : 
    name(name_),
    currentRenderMode(XIAOLIN_WU),
    shader(nullptr)
{
}

void Mesh::translate(const glm::vec3& trans) {
    modelMatrix = glm::translate(modelMatrix, trans);
}

void Mesh::rotate(float angle, const glm::vec3& axis) {
    modelMatrix = glm::rotate(modelMatrix, glm::radians(angle), axis);
}

void Mesh::scale(const glm::vec3& scaleVec) {
    modelMatrix = glm::scale(modelMatrix, scaleVec);
}

void Mesh::resetTransformations() {
    modelMatrix = glm::mat4(1.0f); 
}

bool Mesh::loadFromOBJ(const std::string& filename) {
    points = loadOBJPoints(filename);
    face_indices = loadOBJFaces(filename);
    if(points.empty() || face_indices.empty()){
        std::cerr << "Warning: Mesh loading resulted in empty points or faces." << std::endl;
        return false;
    }
    std::cout << "Loaded " << points.size() << " vertices and " << face_indices.size() << " faces from " << filename << std::endl;
    return true;
}

void Mesh::buildHalfEdge() {
    buildHalfEdgeMeshFromPointsAndFaces(points, face_indices, verticesHE, halfedgesHE, facesHE);
}

void Mesh::setRenderMode(RenderMode newMode) {
    currentRenderMode = newMode;
}

void Mesh::setupMesh() {
    if (halfedgesHE.empty()) {
        std::cerr << "Cannot setup mesh for rendering: half-edge structure not built." << std::endl;
        return;
    }

    edge_indices.clear();

    // Collect unique edges from Half-Edge structure
    for (const auto& he : halfedgesHE) {
        if (he.twin != nullptr && &he > he.twin) {
            continue; // Skip twin to avoid duplicates
        }
        unsigned int idx1 = he.origin - &verticesHE[0];
        unsigned int idx2 = he.next->origin - &verticesHE[0];
        edge_indices.push_back({idx1, idx2});
    }

    glGenVertexArrays(1, &VAO_wu);
    glGenBuffers(1, &VBO_wu);

    glBindVertexArray(VAO_wu);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_wu);

    // Allocating initial size (2 million vertices buffer)
    wu_vbo_allocated_size = 2000000;
    glBufferData(GL_ARRAY_BUFFER, wu_vbo_allocated_size * sizeof(WuVertex), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute (vec2)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(WuVertex), (void*)offsetof(WuVertex, position));
    glEnableVertexAttribArray(0);
    // Color attribute (vec4)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(WuVertex), (void*)offsetof(WuVertex, color));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void Mesh::drawWithXiaolinWu(Shader* shader,
    const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight, const glm::vec4& lineColor, const ViewportRect& viewport) {
    // Clear CPU buffer
    wu_vertex_buffer.clear();

    // 1. Project all mesh vertices to screen space
    std::vector<glm::vec2> screenVerts = projectToScreenSpace(model, view, projection, screenWidth, screenHeight);

    float xMin = static_cast<float>(viewport.x_min);
    float yMin = static_cast<float>(viewport.y_min);
    float xMax = static_cast<float>(viewport.x_max);
    float yMax = static_cast<float>(viewport.y_max);
    WA_Viewport vp{xMin, yMin, xMax, yMax};

    // For each face, clip the polygon and draw the result
    for (const auto& face : face_indices) {
        // Build polygon in screen space
        WA_Polygon poly;
        for (int idx : face) {
            glm::vec2 pt = screenVerts[idx];
            poly.emplace_back(pt.x, pt.y);
        }
        // Clip polygon
        WA_ClipResult clipResult = weiler_atherton_clip(poly, vp);
        // Draw clipped polygon edges
        const WA_Polygon& clipped = clipResult.clipped;
        for (size_t i = 0; i < clipped.size(); ++i) {
            const WA_Point& a = clipped[i];
            const WA_Point& b = clipped[(i+1)%clipped.size()];
            std::vector<Pixel> pixels = drawWuLine2D(glm::vec2(a.x, a.y), glm::vec2(b.x, b.y));
            for (const auto& px : pixels) {
                if (px.intensity > 0.05) {
                    WuVertex v;
                    v.position = glm::vec2(px.x, px.y);
                    v.color = glm::vec4(lineColor.r, lineColor.g, lineColor.b, px.intensity);
                    wu_vertex_buffer.push_back(v);
                }
            }
        }
        // Optionally: draw boundary segments as magenta
        for (const auto& seg : clipResult.boundary_segments) {
            glm::vec2 c1(seg.first.x, seg.first.y);
            glm::vec2 c2(seg.second.x, seg.second.y);
            std::cout << "[DEBUG] Drawing boundary segment: (" << c1.x << ", " << c1.y << ") to (" << c2.x << ", " << c2.y << ")\n";
            std::vector<Pixel> pixels = drawWuLine2D(c1, c2);
            for (const auto& px : pixels) {
                if (px.intensity > 0.05) {
                    WuVertex v;
                    v.position = glm::vec2(px.x, px.y);
                    v.color = glm::vec4(1.0f, 0.0f, 1.0f, px.intensity);
                    wu_vertex_buffer.push_back(v);
                }
            }
        }
    }
    
    // 4. Update GPU
    if (!wu_vertex_buffer.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO_wu);
        
        if (wu_vertex_buffer.size() > wu_vbo_allocated_size) {
            wu_vbo_allocated_size = wu_vertex_buffer.size() * 1.5;
            glBufferData(GL_ARRAY_BUFFER, wu_vbo_allocated_size * sizeof(WuVertex), wu_vertex_buffer.data(), GL_DYNAMIC_DRAW);
        } else {
            glBufferSubData(GL_ARRAY_BUFFER, 0, wu_vertex_buffer.size() * sizeof(WuVertex), wu_vertex_buffer.data());
        }
        wu_point_count = wu_vertex_buffer.size();
    } else {
        wu_point_count = 0;
    }
    
    // 5. Render
    if (wu_point_count > 0) {
        shader->activate();
        shader->setVec2("u_screenSize", {screenWidth, screenHeight});

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST); // Disable depth to ensure markers draw on top of everything

        glBindVertexArray(VAO_wu);
        glDrawArrays(GL_POINTS, 0, wu_point_count);
        glBindVertexArray(0);

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}

// Project all mesh vertices to screen space
std::vector<glm::vec2> Mesh::projectToScreenSpace(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, int screenWidth, int screenHeight) {
    std::vector<glm::vec2> projected;
    for (const auto& v : verticesHE) {
        glm::vec3 world = glm::vec3(model * glm::vec4(v.x, v.y, v.z, 1.0f));
        glm::vec2 screen = projectWorldToScreen(world, view, projection, screenWidth, screenHeight);
        projected.push_back(screen);
    }
    return projected;
}

// Clip all mesh edges to the viewport, return a list of visible edge segments (in screen space)
Mesh::ClipResult Mesh::clipToViewport(const std::vector<glm::vec2>& screenVerts, const ViewportRect& viewport) {
    float xMin = static_cast<float>(viewport.x_min);
    float yMin = static_cast<float>(viewport.y_min);
    float xMax = static_cast<float>(viewport.x_max);
    float yMax = static_cast<float>(viewport.y_max);
    WA_Viewport vp{xMin, yMin, xMax, yMax};
    ClipResult result;
    for (const auto& edge : edge_indices) {
        glm::vec2 p1 = screenVerts[edge.first];
        glm::vec2 p2 = screenVerts[edge.second];
        WA_Polygon seg = { WA_Point(p1.x, p1.y), WA_Point(p2.x, p2.y) };
        WA_ClipResult clipResult = weiler_atherton_clip(seg, vp);
        // Collect visible edges
        const WA_Polygon& clipped = clipResult.clipped;
        for (size_t i = 0; i + 1 < clipped.size(); ++i) {
            glm::vec2 c1(clipped[i].x, clipped[i].y);
            glm::vec2 c2(clipped[i+1].x, clipped[i+1].y);
            result.visibleEdges.emplace_back(c1, c2);
        }
        // Collect boundary segments
        for (const auto& bseg : clipResult.boundary_segments) {
            result.boundarySegments.emplace_back(
                glm::vec2(bseg.first.x, bseg.first.y),
                glm::vec2(bseg.second.x, bseg.second.y)
            );
        }
    }
    return result;
}
