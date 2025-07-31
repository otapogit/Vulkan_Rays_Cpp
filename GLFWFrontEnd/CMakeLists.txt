
# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.3.8
)
FetchContent_MakeAvailable(glfw)

# Assimp 
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG        v5.3.1  # o la versión que prefieras
)
FetchContent_MakeAvailable(assimp)

add_executable(GLFWFrontEnd src/main.cpp)

file (GLOB SOURCES "src/*.cpp" "include/*.h" "include/*.hpp")
target_sources(GLFWFrontEnd PRIVATE ${SOURCES})

target_include_directories(GLFWFrontEnd 
	PRIVATE include 
	#PRIVATE ${pgupv_SOURCE_DIR}/PGUPV/include
)

# PGUPV 
#FetchContent_Declare(
#    pgupv
#    GIT_REPOSITORY https://github.com/fjabad/pgupv.git
#    GIT_TAG        master  # o main, dependiendo de la rama principal
#)

# Hacer disponible PGUPV
#FetchContent_MakeAvailable(pgupv)


    


target_link_libraries(GLFWFrontEnd
    PRIVATE VulkanRenderer
    PRIVATE GLRenderer
    PRIVATE glfw
    #PRIVATE PGUPV
    PRIVATE assimp
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