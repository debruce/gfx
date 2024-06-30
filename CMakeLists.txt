cmake_minimum_required(VERSION 3.29.6)

project(my_opengl VERSION 0.1.0 LANGUAGES CXX)

#include(GNUInstallDirs)
include(CMakeDependentOption)
include_directories(deps)

cmake_dependent_option(GLFW_BUILD_COCOA "Build support for Cocoa" ON "APPLE" OFF)

list(APPEND CMAKE_MODULE_PATH "${GLFW_SOURCE_DIR}/CMake/modules")

find_package(Threads REQUIRED)
find_package(glfw3 3.4 REQUIRED)


add_executable(triangle-opengl src/triangle-opengl.cpp)
#add_executable(triangle-opengl triangle-opengl.c ${ICON} ${GLAD_GL})
target_link_libraries(triangle-opengl glfw)
