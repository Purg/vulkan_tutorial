project(myengine)

################################################################################
# Headers
set(myengine_headers_public
    logging.h
    vulkan/instance.h
    )
set(myengine_headers_private)
source_group("Header Files\\Public"  FILES ${myengine_headers_public})
source_group("Header Files\\Private" FILES ${myengine_headers_private})
install(
  FILES ${myengine_headers_public}
  DESTINATION include/myegine
)

################################################################################
# Source files
set(myengine_source
    vulkan/instance.cxx
    )

################################################################################
# The library target
add_library(
  myengine
  ${myengine_headers_public}
  ${myengine_source}
)
target_link_libraries(
  myengine
  PUBLIC ${PROJECT_LIBRARIES}
  )
