# --- external/CMakeLists.txt ---

# Create a library from the glad source file
# We name it 'glad_lib' for clarity
add_library(
    glad_lib STATIC
    glad/glad.c
)


add_library(
    imgui_lib STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_glfw.cpp
)

# Specify that any target linking against 'glad_lib'
# must also include the current directory to find the glad headers.
target_include_directories(
    glad_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_include_directories(
    imgui_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)