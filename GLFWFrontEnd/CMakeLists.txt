
# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

add_executable(GLFWFrontEnd src/main.cpp)

target_include_directories(GLFWFrontEnd 
	PRIVATE include 
	#PRIVATE ${pgupv_SOURCE_DIR}/PGUPV/include
)

# PGUPV - Descargar desde GitHub
#FetchContent_Declare(
#    pgupv
#    GIT_REPOSITORY https://github.com/fjabad/pgupv.git
#    GIT_TAG        master  # o main, dependiendo de la rama principal
#)

# Hacer disponible PGUPV
#FetchContent_MakeAvailable(pgupv)

find_library(PGUPV_LIB
    NAMES PGUPVd PGUPV pgupvd pgupv
    PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../_deps/PGUPV/
    NO_DEFAULT_PATH
)

    


target_link_libraries(GLFWFrontEnd
    PRIVATE VulkanRenderer
    PRIVATE GLRenderer
    PRIVATE glfw
    PRIVATE ${PGUPV_LIB}
)



set_target_properties(GLFWFrontEnd PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    #CXX_STANDARD 17
    #CXX_STANDARD_REQUIRED ON
)

target_compile_features(GLFWFrontEnd PRIVATE cxx_std_17)

target_link_libraries(GLFWFrontEnd PRIVATE 
        opengl32
        glu32
)