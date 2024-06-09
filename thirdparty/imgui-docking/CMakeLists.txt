cmake_minimum_required(VERSION 3.1)
project(imgui)

add_library(imgui)
file(GLOB_RECURSE IMGUI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
target_sources(imgui PRIVATE "${IMGUI_SOURCES}")

target_include_directories(imgui PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")


target_link_libraries(imgui PRIVATE glfw)