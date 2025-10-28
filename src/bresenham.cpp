#include "mesh.hpp"
#include <vector>
#include <glm/glm.hpp>

// Bresenham's line algorithm for 2D lines with 3D projection to screen
std::vector<glm::ivec2> bresenhamLine(const glm::vec3& p0, const glm::vec3& p1, int width, int height, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model) {
    // Project 3D points to 2D screen coordinates
    glm::vec4 p0_clip = projection * view * model * glm::vec4(p0, 1.0f);
    glm::vec4 p1_clip = projection * view * model * glm::vec4(p1, 1.0f);
    p0_clip /= p0_clip.w;
    p1_clip /= p1_clip.w;
    // Convert to screen coordinates
    int x0 = int((p0_clip.x * 0.5f + 0.5f) * width);
    int y0 = int((1.0f - (p0_clip.y * 0.5f + 0.5f)) * height);
    int x1 = int((p1_clip.x * 0.5f + 0.5f) * width);
    int y1 = int((1.0f - (p1_clip.y * 0.5f + 0.5f)) * height);

    std::vector<glm::ivec2> points;
    int dx = abs(x1 - x0);
    int dy = abs(y1 - y0);
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;

    while (true) {
        points.emplace_back(x0, y0);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
    return points;
}
