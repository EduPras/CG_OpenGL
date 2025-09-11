# --- Top-Level CMakeLists.txt ---

# Define the minimum CMake version and project name
cmake_minimum_required(VERSION 3.10.0)
project(LearnOpenGl VERSION 0.1.0 LANGUAGES C CXX)

# Find necessary libraries
# GLFW is almost always needed alongside GLAD and OpenGL
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Process the other CMakeLists.txt files in their respective directories
add_subdirectory(external)
add_subdirectory(src)