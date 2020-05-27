add_executable(enumerate_vulkan_ext_properties
               enumerate_vulkan_ext_properties.cpp)
target_link_libraries(enumerate_vulkan_ext_properties
                      PRIVATE ${PROJECT_LIBRARIES})
