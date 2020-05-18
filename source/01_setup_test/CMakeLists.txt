find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(vkt_vulkan_test main.cpp)
target_link_libraries(vkt_vulkan_test PUBLIC glm glfw ${Vulkan_LIBRARIES})
