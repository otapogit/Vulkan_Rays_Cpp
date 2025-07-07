add_executable(CLIFrontEnd src/main.cpp)

target_link_libraries(CLIFrontEnd
    PRIVATE VulkanRenderer
    PRIVATE GLRenderer
)

target_compile_features(CLIFrontEnd PRIVATE cxx_std_17)
