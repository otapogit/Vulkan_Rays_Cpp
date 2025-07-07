add_executable(GLFWFrontEnd src/main.cpp)

target_link_libraries(GLFWFrontEnd
    PRIVATE VulkanRenderer
    PRIVATE GLRenderer
    PRIVATE glfw
)

target_compile_features(GLFWFrontEnd PRIVATE cxx_std_17)
