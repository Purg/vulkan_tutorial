find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(hello_triangle
        hello_triangle.cpp
        )
target_link_libraries(hello_triangle
        PUBLIC glm glfw ${Vulkan_LIBRARIES}
        )
