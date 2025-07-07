file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")

add_library(Renderer ${SOURCES})

target_include_directories(Renderer PUBLIC include)
target_link_libraries(Renderer PUBLIC glm::glm)
target_compile_features(Renderer PUBLIC cxx_std_17)
