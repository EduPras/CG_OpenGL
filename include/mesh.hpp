#pragma once
#include <vector>
#include <string>
#include "utils.hpp"
#include "half_edge.hpp"

class Mesh {
public:
    std::vector<Point> points;
    std::vector<std::vector<int>> faces;
    std::vector<Vertex> verticesHE;
    std::vector<HalfEdge> halfedgesHE;
    std::vector<Face> facesHE;

    bool loadFromOBJ(const std::string& filename);
    void buildHalfEdge();
};
