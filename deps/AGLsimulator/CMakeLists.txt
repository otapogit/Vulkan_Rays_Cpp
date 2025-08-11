add_library(AGLsimulator STATIC IMPORTED GLOBAL)

# Directorio de cabeceras
set_target_properties(AGLsimulator PROPERTIES
  IMPORTED_CONFIGURATIONS "Debug;Release"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Ruta a los .lib
set_target_properties(AGLsimulator PROPERTIES
  IMPORTED_LOCATION_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/libs/Debug/AGLsimulator.lib"
  IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/libs/Release/AGLsimulator.lib"
)
