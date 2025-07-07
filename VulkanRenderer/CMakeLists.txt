file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")

add_library(VulkanRenderer ${SOURCES})

target_include_directories(VulkanRenderer PUBLIC include)
target_link_libraries(VulkanRenderer
    PUBLIC Renderer
    PRIVATE Vulkan::Vulkan
)
target_compile_features(VulkanRenderer PUBLIC cxx_std_17)

# Find Vulkan
find_package(Vulkan REQUIRED)
