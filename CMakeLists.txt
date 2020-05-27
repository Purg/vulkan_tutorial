cmake_minimum_required(VERSION 3.10.2)
project(vulkan_tutorial VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

# Add the local `cmake` directory to the CMake Module path.
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

# Package Dependencies
# - Next steps: learn from KWIVER, create different "depends" cmake modules
#   to encapsulate package specific include behavior instead of everything just
#   being right here, which is really messy.
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
include_directories(${Vulkan_INCLUDE_DIRS})
list(APPEND PROJECT_LIBRARIES glm glfw ${Vulkan_LIBRARIES})

# Include project header files within sub-directories.
# - Assumes headers will be installed
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}")

# Descend into the source
add_subdirectory(myengine)
add_subdirectory(tools)
