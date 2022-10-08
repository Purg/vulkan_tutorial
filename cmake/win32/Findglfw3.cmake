# Find the GLFW3 library and include directory
#
# The "glm_DIR" variable optionally specifies a GLM installation root
# directory.

mark_as_advanced( glfw3_DIR )

find_path( glfw3_INCLUDE_DIR
  NAMES GLFW/glfw3.h
  HINTS "${glfw3_DIR}/include" )

# Different library directories depending on the Visual Studio version
if( MSVC_VERSION GREATER_EQUAL "1930" AND MSVC_VERSION LESS "1940" )
  MESSAGE( STATUS "Detected MSVC 2022" )
  set( glfw3_PATH_SUFFIXES "lib-vc2022" )
else()
  message( FATAL_ERROR "Missing logic to handle MSVC version: ${MSVC_VERSION}" )
endif()
# On Windows, the library name to seek is different depending on whether we are
# building statically ("glfw3") or shared ("glfw3dll").
set( glfw3_lib_name "glfw3" )
if( MSVC AND BUILD_SHARED_LIBS )
  set( glfw3_lib_name "glfw3dll" )
endif()
find_library( glfw3_LIBRARY "${glfw3_lib_name}"
  HINTS "${glfw3_DIR}"
  PATH_SUFFIXES "${glfw3_PATH_SUFFIXES}"
  REQUIRED )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( glfw3
  REQUIRED_VARS glfw3_INCLUDE_DIR glfw3_LIBRARY
  )

if( glfw3_FOUND AND NOT TARGET glfw )
  add_library( glfw UNKNOWN IMPORTED )
  set_target_properties( glfw PROPERTIES
    IMPORTED_LOCATION "${glfw3_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${glfw3_INCLUDE_DIR}" )
endif()
