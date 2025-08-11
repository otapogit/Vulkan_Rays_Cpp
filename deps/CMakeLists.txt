
# FetchContent for external deps
include(FetchContent)

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
)
FetchContent_MakeAvailable(glm)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)


add_subdirectory(pugixml)
add_subdirectory(AGL)
add_subdirectory(AGLcommon)
add_subdirectory(AGLcommonUtils)
add_subdirectory(AGLhelper)
add_subdirectory(AGLimport)
add_subdirectory(AGLmath)
add_subdirectory(AGLwin32)
add_subdirectory(AGLsimulator)