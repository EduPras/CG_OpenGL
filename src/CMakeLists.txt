# --- src/CMakeLists.txt ---

# Define the executable target and its source files
add_executable(
    LearnOpenGl
    main.cpp
    shader.cpp
    utils.cpp
    half_edge.cpp
    mesh.cpp
    viewer.cpp
    input.cpp
)


# Add include directory for headers
target_include_directories(LearnOpenGl PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Link the executable against the libraries it needs:
target_link_libraries(
    LearnOpenGl PRIVATE
    glad_lib
    glfw
    OpenGL::GL
)

file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})