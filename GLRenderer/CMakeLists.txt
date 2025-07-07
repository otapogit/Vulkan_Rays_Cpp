file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")

add_library(GLRenderer ${SOURCES})

target_include_directories(GLRenderer PUBLIC include)
target_link_libraries(GLRenderer
    PUBLIC Renderer
    PRIVATE OpenGL::GL
)
target_compile_features(GLRenderer PUBLIC cxx_std_17)

# Find OpenGL
find_package(OpenGL REQUIRED)
