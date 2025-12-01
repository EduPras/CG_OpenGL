#pragma once
#include <vector>
#include <glm/glm.hpp>

struct WA_Point {
    float x, y;
    WA_Point(float x_, float y_) : x(x_), y(y_) {}
};
using WA_Polygon = std::vector<WA_Point>;

struct WA_Viewport {
    float xmin, ymin, xmax, ymax;
};

// Holds both the clipped polygon and the intersection segments on the viewport boundary
struct WA_ClipResult {
    WA_Polygon clipped;
    std::vector<std::pair<WA_Point, WA_Point>> boundary_segments;
};

WA_ClipResult weiler_atherton_clip(const WA_Polygon& poly, const WA_Viewport& vp);
