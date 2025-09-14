#pragma once
#include <vector>
#include "utils.hpp"

void processInput(GLFWwindow *window, int total_segments, int visible_vertices, std::vector<Point>& data_points, float& zoom_level, float& rotation_angle, float& pan_x, float& pan_y);
