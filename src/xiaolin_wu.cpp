#include <vector>
#include <cmath>
#include <algorithm> // For std::swap
#include <glm/glm.hpp>
#include "xiaolin_wu.hpp"

std::vector<Pixel> drawWuLine2D(glm::vec2 p0, glm::vec2 p1) {
    std::vector<Pixel> pixels;
    bool steep = std::abs(p1.y - p0.y) > std::abs(p1.x - p0.x);

    // Swap (x, y) for steep lines to handle them as if they were shallow
    if (steep) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
    }

    // Make sure we are always drawing from left to right
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }

    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;
    float gradient = (dx == 0.0f) ? 1.0f : dy / dx;

    // --- Handle the first endpoint ---
    int x_start = round(p0.x);
    double y_start = p0.y + gradient * (x_start - p0.x);
    
    int x_start_px = x_start;
    int y_start_px = floor(y_start);

    if (steep) {
        pixels.push_back({y_start_px,     x_start_px, rfpart(y_start)});
        pixels.push_back({y_start_px + 1, x_start_px,  fpart(y_start)});
    } else {
        pixels.push_back({x_start_px, y_start_px,     rfpart(y_start)});
        pixels.push_back({x_start_px, y_start_px + 1,  fpart(y_start)});
    }
    
    // --- Main loop along the major axis ---
    int x_end = round(p1.x);
    double y = y_start + gradient; // First y-position after the starting endpoint

    for (int x = x_start + 1; x < x_end; ++x) {
        int ix = floor(y);
        if (steep) {
            pixels.push_back({ix,     x, rfpart(y)});
            pixels.push_back({ix + 1, x,  fpart(y)});
        } else {
            pixels.push_back({x, ix,     rfpart(y)});
            pixels.push_back({x, ix + 1,  fpart(y)});
        }
        y += gradient;
    }

    // --- Handle the second endpoint ---
    double y_end = p1.y + gradient * (x_end - p1.x);
    int x_end_px = x_end;
    int y_end_px = floor(y_end);

    if (steep) {
        pixels.push_back({y_end_px,     x_end_px, rfpart(y_end)});
        pixels.push_back({y_end_px + 1, x_end_px,  fpart(y_end)});
    } else {
        pixels.push_back({x_end_px, y_end_px,     rfpart(y_end)});
        pixels.push_back({x_end_px, y_end_px + 1,  fpart(y_end)});
    }

    return pixels;
}