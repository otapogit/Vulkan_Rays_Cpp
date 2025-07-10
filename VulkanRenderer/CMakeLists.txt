# GLSLang (GLSL to SPIR-V compiler)
FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG        main
)

set (ENABLE_OPT "0")
set(ENABLE_GLSLANG_BINARIES "OFF")

FetchContent_MakeAvailable(glslang)

file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp" "src/Renderer/*.cpp" "src/core/*.cpp" "include/core/*.h" "include/Renderer/*.h" "include/Renderer/*.hpp" "include/core/*.hpp")
add_library(VulkanRenderer ${SOURCES})

target_include_directories(VulkanRenderer PUBLIC include)
target_link_libraries(VulkanRenderer
    PUBLIC Renderer
    PRIVATE Vulkan::Vulkan
	PRIVATE glslang
)
target_compile_features(VulkanRenderer PUBLIC cxx_std_17)

# Find Vulkan
find_package(Vulkan REQUIRED)
