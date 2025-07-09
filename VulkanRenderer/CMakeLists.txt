# Find required packages
find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)

# Configure GLM manually to avoid conflicts
set(GLM_INCLUDE_DIR "C:/Users/carlos/source/repos/glm" CACHE PATH "GLM include directory")
if(NOT TARGET glm::glm)
    add_library(glm_interface INTERFACE)
    target_include_directories(glm_interface INTERFACE ${GLM_INCLUDE_DIR})
    add_library(glm::glm ALIAS glm_interface)
endif()

# Try to find glslang
find_package(glslang QUIET)
if(NOT glslang_FOUND)
    # If not found, try to find it as a subdirectory or fallback
    find_path(GLSLANG_INCLUDE_DIR glslang/Public/ShaderLang.h)
    find_library(GLSLANG_LIBRARY NAMES glslang)
    find_library(GLSLANG_SPIRV_LIBRARY NAMES SPIRV)
    find_library(GLSLANG_OSDEPENDENT_LIBRARY NAMES OSDependent)
    find_library(GLSLANG_OGLCOMPILER_LIBRARY NAMES OGLCompiler)
    
    if(GLSLANG_INCLUDE_DIR AND GLSLANG_LIBRARY)
        set(GLSLANG_LIBRARIES 
            ${GLSLANG_LIBRARY}
            ${GLSLANG_SPIRV_LIBRARY}
            ${GLSLANG_OSDEPENDENT_LIBRARY}
            ${GLSLANG_OGLCOMPILER_LIBRARY}
        )
    endif()
endif()

file(GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")
add_library(VulkanRenderer ${SOURCES})

target_include_directories(VulkanRenderer 
    PUBLIC include
    PRIVATE 
        ${GLSLANG_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
)

target_link_libraries(VulkanRenderer
    PUBLIC 
        Renderer
        glm::glm
    PRIVATE 
        Vulkan::Vulkan
        ${GLSLANG_LIBRARIES}
)

target_compile_features(VulkanRenderer PUBLIC cxx_std_17)
