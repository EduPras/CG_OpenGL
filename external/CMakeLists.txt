# --- external/CMakeLists.txt ---

# Create a library from the glad source file
# We name it 'glad_lib' for clarity
add_library(
    glad_lib STATIC
    glad/glad.c
)

# Specify that any target linking against 'glad_lib'
# must also include the current directory to find the glad headers.
target_include_directories(
    glad_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)