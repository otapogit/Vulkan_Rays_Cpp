file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")

add_library(Renderer ${SOURCES})

include(FetchContent)

FetchContent_Declare(
  tinyobjloader
  GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
  GIT_TAG        release # o la versión que desees
)

FetchContent_MakeAvailable(tinyobjloader)

target_link_libraries(Renderer PRIVATE tinyobjloader)


target_include_directories(Renderer PUBLIC include)
target_link_libraries(Renderer PUBLIC glm::glm)
target_compile_features(Renderer PUBLIC cxx_std_17)
