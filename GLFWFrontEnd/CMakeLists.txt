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

set_target_properties(GLFWFrontEnd PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

target_compile_features(GLFWFrontEnd PRIVATE cxx_std_17)

