cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build mode")

project(initiation)

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Set compile options
add_compile_options(-std=c++17 -pthread)

add_subdirectory(src)