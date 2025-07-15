# Find Vulkan
find_package(Vulkan REQUIRED)

# Find OpenGL (need to export the image)
find_package(OpenGL REQUIRED)

# Find GLEW para extensiones OpenGL
    FetchContent_Declare(
        glew
        URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0-win32.zip
    )
    FetchContent_MakeAvailable(glew)
    
    # Configurar GLEW manualmente
    set(GLEW_INCLUDE_DIRS ${glew_SOURCE_DIR}/include)
    set(GLEW_LIBRARIES ${glew_SOURCE_DIR}/lib/Release/x64/glew32s.lib)
    
    # Crear target si no existe
    if(NOT TARGET GLEW::GLEW)
        add_library(GLEW::GLEW INTERFACE IMPORTED)
        set_target_properties(GLEW::GLEW PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIRS}"
            INTERFACE_LINK_LIBRARIES "${GLEW_LIBRARIES}"
        )
    endif()

# GLSLang (GLSL to SPIR-V compiler)
FetchContent_Declare(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG        main
)

set (ENABLE_OPT "0")
set(ENABLE_GLSLANG_BINARIES "OFF")

FetchContent_MakeAvailable(glslang)

file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp" "src/Renderer/*.cpp" "src/core/*.cpp" "include/core/*.h" "include/Renderer/*.h" "include/Renderer/*.hpp" "include/core/*.hpp" "Shaders/*.rchit" "Shaders/*.rmiss" "Shaders/*.rgen")
add_library(VulkanRenderer ${SOURCES})

target_include_directories(VulkanRenderer PUBLIC include
PUBLIC ${OPENGL_INCLUDE_DIRS}
PUBLIC ${GLEW_INCLUDE_DIRS})
target_link_libraries(VulkanRenderer
    PUBLIC Renderer
    PRIVATE Vulkan::Vulkan
	PRIVATE glslang
	PRIVATE  glslang-default-resource-limits
	PRIVATE OpenGL::GL
	PRIVATE GLEW::GLEW
)
target_compile_features(VulkanRenderer PUBLIC cxx_std_17)


