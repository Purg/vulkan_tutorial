cmake_minimum_required(VERSION 3.10.2)
project(vulkan_tutorial VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Graphic library dependencies
find_package(glm REQUIRED)

find_package(glfw3 REQUIRED)

find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})

list(APPEND PROJECT_LIBRARIES glm glfw ${Vulkan_LIBRARIES})
message(STATUS "Graphics libraries: ${PROJECT_LIBRARIES}")

# Descend into the source
add_subdirectory(source)
