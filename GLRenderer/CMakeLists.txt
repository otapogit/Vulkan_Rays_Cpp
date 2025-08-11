# Find OpenGL
find_package(OpenGL REQUIRED)

file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")

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

add_library(GLRenderer ${SOURCES})

target_include_directories(GLRenderer PUBLIC include)
target_link_libraries(GLRenderer
    PUBLIC Renderer
    PRIVATE OpenGL::GL GLEW::GLEW AGL AGLimport AGLcommonUtils AGLcommon AGLhelper AGLwin32 AGLmath AGLsimulator pugixml
)
target_compile_features(GLRenderer PUBLIC cxx_std_17)


