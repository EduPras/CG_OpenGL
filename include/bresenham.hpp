/**
 * @file bresenham.hpp
 * @brief Implements Bresenham's line drawing algorithm for 2D lines with 3D projection.
 */
#pragma once
#include <vector>
#include <glm/glm.hpp>

// Returns a vector of 2D screen points for a 3D-projected line
std::vector<glm::ivec2> bresenhamLine(const glm::vec3& p0, const glm::vec3& p1, int width, int height, const glm::mat4& projection, const glm::mat4& view, const glm::mat4& model);
