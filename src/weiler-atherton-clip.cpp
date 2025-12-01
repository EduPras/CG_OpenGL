// Weiler-Atherton polygon clipping algorithm implementation
// This is a simplified version for convex viewport clipping
#include <vector>
#include <glm/glm.hpp>


// A point in 2D
struct WA_Point {
    float x, y;
    WA_Point(float x_, float y_) : x(x_), y(y_) {}
};
// A polygon is a list of points
using WA_Polygon = std::vector<WA_Point>;
// The viewport is defined as a rectangle (xmin, ymin, xmax, ymax)
struct WA_Viewport {
    float xmin, ymin, xmax, ymax;
};

// Holds both the clipped polygon and the intersection segments on the viewport boundary
struct WA_ClipResult {
    WA_Polygon clipped;
    std::vector<std::pair<WA_Point, WA_Point>> boundary_segments;
};

// Helper: check if point is inside the viewport
static bool wa_inside(const WA_Point& p, const WA_Viewport& vp) {
    return p.x >= vp.xmin && p.x <= vp.xmax && p.y >= vp.ymin && p.y <= vp.ymax;
}

// Helper: compute intersection between polygon edge and viewport edge
static WA_Point wa_intersect(const WA_Point& p1, const WA_Point& p2, float edge, bool vertical) {
    float x, y;
    if (vertical) {
        x = edge;
        y = p1.y + (p2.y - p1.y) * (edge - p1.x) / (p2.x - p1.x);
    } else {
        y = edge;
        x = p1.x + (p2.x - p1.x) * (edge - p1.y) / (p2.y - p1.y);
    }
    return WA_Point(x, y);
}

// Clip a polygon against a single edge
static WA_Polygon wa_clip_edge(const WA_Polygon& poly, float edge, bool vertical, bool inside_less) {
    WA_Polygon out;
    size_t n = poly.size();
    for (size_t i = 0; i < n; ++i) {
        const WA_Point& curr = poly[i];
        const WA_Point& prev = poly[(i + n - 1) % n];
        bool curr_in = vertical ? (inside_less ? curr.x >= edge : curr.x <= edge)
                                : (inside_less ? curr.y >= edge : curr.y <= edge);
        bool prev_in = vertical ? (inside_less ? prev.x >= edge : prev.x <= edge)
                                : (inside_less ? prev.y >= edge : prev.y <= edge);
        if (curr_in) {
            if (!prev_in) {
                out.push_back(wa_intersect(prev, curr, edge, vertical));
            }
            out.push_back(curr);
        } else if (prev_in) {
            out.push_back(wa_intersect(prev, curr, edge, vertical));
        }
    }
    return out;
}

// Main Weiler-Atherton clipping function
// Returns both the clipped polygon and the intersection segments on the viewport boundary
WA_ClipResult weiler_atherton_clip(const WA_Polygon& poly, const WA_Viewport& vp) {
    WA_Polygon out = poly;
    std::vector<std::pair<WA_Point, WA_Point>> boundary_segments;

    // Helper lambda to collect boundary segments as a polyline
    auto collect_boundary = [&](const WA_Polygon& poly, float edge, bool vertical) {
        std::vector<WA_Point> boundary_points;
        for (const auto& pt : poly) {
            if (vertical) {
                if (std::abs(pt.x - edge) < 1e-3) boundary_points.push_back(pt);
            } else {
                if (std::abs(pt.y - edge) < 1e-3) boundary_points.push_back(pt);
            }
        }
        // Add as consecutive segments
        for (size_t i = 0; i + 1 < boundary_points.size(); ++i) {
            boundary_segments.emplace_back(boundary_points[i], boundary_points[i + 1]);
        }
    };

        // Special handling for line segments (2-point input)
        if (poly.size() == 2) {
            WA_Polygon clipped = poly;
            clipped = wa_clip_edge(clipped, vp.xmin, true, true);
            clipped = wa_clip_edge(clipped, vp.xmax, true, false);
            clipped = wa_clip_edge(clipped, vp.ymin, false, true);
            clipped = wa_clip_edge(clipped, vp.ymax, false, false);
            // Check if both endpoints are on the same boundary (vertical or horizontal)
            if (clipped.size() == 2) {
                const WA_Point& a = clipped[0];
                const WA_Point& b = clipped[1];
                // Left
                if (std::abs(a.x - vp.xmin) < 1e-3 && std::abs(b.x - vp.xmin) < 1e-3)
                    boundary_segments.emplace_back(a, b);
                // Right
                else if (std::abs(a.x - vp.xmax) < 1e-3 && std::abs(b.x - vp.xmax) < 1e-3)
                    boundary_segments.emplace_back(a, b);
                // Bottom
                else if (std::abs(a.y - vp.ymin) < 1e-3 && std::abs(b.y - vp.ymin) < 1e-3)
                    boundary_segments.emplace_back(a, b);
                // Top
                else if (std::abs(a.y - vp.ymax) < 1e-3 && std::abs(b.y - vp.ymax) < 1e-3)
                    boundary_segments.emplace_back(a, b);
            }
            return {clipped, boundary_segments};
        }
        // Fallback for polygons (not used in your case)
        out = wa_clip_edge(out, vp.xmin, true, true);
        out = wa_clip_edge(out, vp.xmax, true, false);
        out = wa_clip_edge(out, vp.ymin, false, true);
        out = wa_clip_edge(out, vp.ymax, false, false);
        return {out, boundary_segments};

    return {out, boundary_segments};
}
