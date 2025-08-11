add_executable(CLIFrontEnd src/main.cpp)

target_link_libraries(CLIFrontEnd
    PRIVATE VulkanRenderer
    PRIVATE GLRenderer
)

set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/data")

file(GLOB ASSET_FILES RELATIVE ${ASSETS_SRC_DIR} "${ASSETS_SRC_DIR}/*")

foreach(file ${ASSET_FILES})
    add_custom_command(TARGET CLIFrontEnd POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ASSETS_SRC_DIR}/${file}" $<TARGET_FILE_DIR:CLIFrontEnd>
    )
endforeach()

target_compile_features(CLIFrontEnd PRIVATE cxx_std_17)
