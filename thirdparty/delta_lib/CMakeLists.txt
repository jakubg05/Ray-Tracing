cmake_minimum_required(VERSION 3.1)
project(delta_lib)

add_library(delta_lib INTERFACE)


target_include_directories(delta_lib INTERFACE include "${CMAKE_CURRENT_SOURCE_DIR}/")


target_link_libraries(delta_lib INTERFACE glfw)