add_executable( vkt_vulkan_test main.cpp )
target_link_libraries( vkt_vulkan_test PRIVATE glm glfw Vulkan::Vulkan )
