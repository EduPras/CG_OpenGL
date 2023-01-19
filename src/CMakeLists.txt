find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)

add_library(
    shader
    shader.hpp
    shader.cpp
)

add_executable(LearnOpenGl main.cpp)
target_link_libraries(LearnOpenGl 
    PRIVATE
        ${OPENGL_LIBRARIES} 
        glm
        glfw
    PUBLIC
        external
        shader
)
