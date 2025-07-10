# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

add_executable(GLFWFrontEnd src/main.cpp)

target_link_libraries(GLFWFrontEnd
    PRIVATE VulkanRenderer
    PRIVATE GLRenderer
    PRIVATE glfw
)

target_compile_features(GLFWFrontEnd PRIVATE cxx_std_17)

