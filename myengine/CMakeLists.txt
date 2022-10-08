project( myengine )

####################################################################################################
# Headers
set( myengine_headers_public
  glfw.h
  logging.h
  vulkan.h
  )
source_group( "Header Files\\Public" FILES ${myengine_headers_public} )
install(
  FILES ${myengine_headers_public}
  DESTINATION include/myengine
)

####################################################################################################
# Source files
set( myengine_source
  glfw.cxx
  logging.cxx
  vulkan.cxx )

####################################################################################################
# The library target
add_library( myengine
  ${myengine_headers_public}
  ${myengine_source}
)
target_link_libraries( myengine
  PUBLIC glm glfw Vulkan::Vulkan
  )

include( GenerateExportHeader )
generate_export_header( myengine )
