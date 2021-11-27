add_executable( enumerate_vulkan_ext_properties
  enumerate_vulkan_ext_properties.cxx )
set_target_properties( enumerate_vulkan_ext_properties PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
target_link_libraries( enumerate_vulkan_ext_properties
  PRIVATE myengine
  )
