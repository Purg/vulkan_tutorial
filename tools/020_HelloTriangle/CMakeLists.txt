add_executable( hello_triangle
  hello_triangle.cxx
  )
set_target_properties( hello_triangle PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
# We need to "know" to pass PROJECT_LIBRARIES here.
# This is why KWIVER created that helper "kwiver_add_library" wrapper...
target_link_libraries( hello_triangle
  PRIVATE myengine
  )
