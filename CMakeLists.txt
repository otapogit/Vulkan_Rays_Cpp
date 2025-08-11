cmake_minimum_required(VERSION 3.20)
project(MyProject LANGUAGES CXX)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Subdirectories
add_subdirectory(deps)
add_subdirectory(Renderer)
add_subdirectory(VulkanRenderer)
add_subdirectory(GLRenderer)
add_subdirectory(GLFWFrontEnd)
add_subdirectory(CLIFrontEnd)
