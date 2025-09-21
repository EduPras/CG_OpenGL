#include "mesh.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h> // Make sure to include GLAD

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), line_index_count(0) {}

Mesh::~Mesh() {
    // Destructor to clean up GPU resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

bool Mesh::loadFromOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open OBJ file " << filename << std::endl;
        return false;
    }

    points.clear();
    face_indices.clear();

    std::string line;
    while (std::getline(file, line)) {
        size_t comment_pos = line.find('#');
        std::string content_line = line;
        if (comment_pos != std::string::npos) {
            content_line = line.substr(0, comment_pos);
        }

        std::stringstream ss(content_line);
        std::string keyword;
        ss >> keyword;

        if (keyword == "v") {
            Point p;
            ss >> p.x >> p.y >> p.z;
            points.push_back(p);
        } else if (keyword == "f") {
            std::vector<int> indices;
            std::string face_val;
            while (ss >> face_val) {
                size_t first_slash = face_val.find('/');
                if (first_slash != std::string::npos) {
                    face_val = face_val.substr(0, first_slash);
                }
                try {
                    indices.push_back(std::stoi(face_val) - 1);
                } catch (const std::invalid_argument& e) {
                    std::cerr << "Warning: Could not parse face index value: " << face_val << std::endl;
                }
            }
            if (!indices.empty()) {
                face_indices.push_back(indices);
            }
        }
    }
    std::cout << "Loaded " << points.size() << " vertices and " << face_indices.size() << " faces from " << filename << std::endl;
    return true;
}

void Mesh::buildHalfEdge() {
    buildHalfEdgeMeshFromPointsAndFaces(points, face_indices, verticesHE, halfedgesHE, facesHE);
}

// --- NEW: Setup GPU buffers for rendering ---
void Mesh::setupMesh() {
    // Create line indices from face data for wireframe rendering
    std::vector<unsigned int> line_indices;
    for (const auto& face : face_indices) {
        for (size_t i = 0; i < face.size(); ++i) {
            line_indices.push_back(face[i]);
            line_indices.push_back(face[(i + 1) % face.size()]);
        }
    }
    line_index_count = line_indices.size();

    // 1. Generate and bind VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 2. Copy vertex data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(Point), points.data(), GL_STATIC_DRAW);

    // 3. Copy index data to EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, line_indices.size() * sizeof(unsigned int), line_indices.data(), GL_STATIC_DRAW);

    // 4. Set vertex attribute pointers
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
    glEnableVertexAttribArray(0);

    // 5. Unbind VAO
    glBindVertexArray(0);
}

// --- NEW: Draw the mesh ---
void Mesh::draw() {
    glBindVertexArray(VAO);
    glDrawElements(GL_LINES, line_index_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
