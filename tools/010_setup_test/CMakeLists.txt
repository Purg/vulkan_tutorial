add_executable( vkt_vulkan_test main.cxx )
set_target_properties( vkt_vulkan_test PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
  )
target_link_libraries( vkt_vulkan_test
  PRIVATE glm glfw Vulkan::Vulkan )
