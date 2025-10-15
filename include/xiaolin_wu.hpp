#include <vector>
#include <cmath>
#include <algorithm> // For std::swap
#include <glm/glm.hpp>

// Represents a single pixel to be drawn on the screen.
struct Pixel {
    int x, y;
    double intensity; // Intensity/alpha from 0.0 (transparent) to 1.0 (opaque)
};

// Helper function to get the fractional part of a number.
inline double fpart(double x) {
    return x - floor(x);
}

// Helper function to get the one-complement of the fractional part.
inline double rfpart(double x) {
    return 1.0 - fpart(x);
}

/**
 * @brief Draws an anti-aliased 2D line using Xiaolin Wu's algorithm.
 *
 * @param p0 The starting 2D point (in screen coordinates).
 * @param p1 The ending 2D point (in screen coordinates).
 * @return A vector of Pixel objects representing the line.
 */
std::vector<Pixel> drawWuLine2D(glm::vec2 p0, glm::vec2 p1);