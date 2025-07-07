cmake_minimum_required(VERSION 3.20)
project(MyProject LANGUAGES CXX)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent for external deps
include(FetchContent)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        0.9.9.8
)
FetchContent_MakeAvailable(glm)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# Subdirectories
add_subdirectory(Renderer)
add_subdirectory(VulkanRenderer)
add_subdirectory(GLRenderer)
add_subdirectory(GLFWFrontEnd)
add_subdirectory(CLIFrontEnd)
