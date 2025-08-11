
set(FreeImage_ROOT ${CMAKE_SOURCE_DIR}/deps/FreeImage)

find_package(FreeImage REQUIRED)


add_library(AGL STATIC IMPORTED GLOBAL)

# Directorio de cabeceras
set_target_properties(AGL PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Ruta a los .lib
set_target_properties(AGL PROPERTIES
  IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/libs/Debug/AGL.lib"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/Release/AGL.lib"
)

target_link_libraries(AGL INTERFACE debug ${FreeImage_STATIC_LIBRARY_DEBUG} optimized ${FreeImage_STATIC_LIBRARY})
#target_compile_definitions(AGL PUBLIC FREEIMAGE_LIB)