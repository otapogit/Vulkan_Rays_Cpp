add_library(AGLcommon STATIC IMPORTED GLOBAL)

# Directorio de cabeceras
set_target_properties(AGLcommon PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Ruta a los .lib
set_target_properties(AGLcommon PROPERTIES
  IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/libs/Debug/AGLcommon.lib"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/Release/AGLcommon.lib"
)
