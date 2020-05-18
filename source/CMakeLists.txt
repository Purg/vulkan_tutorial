# Graphic library dependencies
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
list(APPEND PROJECT_LIBRARIES glm glfw ${Vulkan_LIBRARIES})
message(STATUS "Graphics libraries: ${PROJECT_LIBRARIES}")


add_subdirectory(01_setup_test)
add_subdirectory(02_HelloTriangle)
