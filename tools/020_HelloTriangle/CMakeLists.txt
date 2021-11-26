add_executable( hello_triangle
  hello_triangle.cxx
  )
# We need to "know" to pass PROJECT_LIBRARIES here.
# This is why KWIVER created that helper "kwiver_add_library" wrapper...
target_link_libraries( hello_triangle
  PRIVATE myengine
  )
