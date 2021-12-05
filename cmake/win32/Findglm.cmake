# Find the GLM header-only installation and include that directory.
#
# The "glm_DIR" variable optionally specifies a GLM installation root
# directory.

mark_as_advanced( glm_DIR )

find_path( glm_INCLUDE_DIR
  NAMES glm/glm.hpp
  HINTS "${glm_DIR}" )

include( FindPackageHandleStandardArgs )
find_package_handle_standard_args( glm
  REQUIRED_VARS glm_INCLUDE_DIR
  )

if( glm_FOUND AND NOT TARGET glm )
  add_library( glm INTERFACE IMPORTED )
  set_target_properties( glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${glm_INCLUDE_DIR}" )
endif()
