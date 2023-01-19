cmake_minimum_required(VERSION 3.0.0)
project(LearnOpenGl VERSION 0.1.0)

include_directories(
    ${OPENGL_INCLUDE_DIR} 
    ./external
    ${root_SOURCE_DIR}
)

add_subdirectory(external)
add_subdirectory(src)