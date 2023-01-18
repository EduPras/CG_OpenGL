cmake_minimum_required(VERSION 3.0.0)
project(MIT_lessons VERSION 0.1.0)

find_package(OpenGL REQUIRED)
find_package(glfw3 3.3 REQUIRED)

#include(CTest)
#enable_testing()
add_executable(MIT_lessons src/main.cpp src/glad.c)
include_directories(${OPENGL_INCLUDE_DIR} ./include)


target_link_libraries(MIT_lessons ${OPENGL_LIBRARIES} glfw)



#set(CPACK_PROJECT_NAME ${PROJECT_NAME})
#set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)
