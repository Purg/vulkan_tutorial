cmake_minimum_required( VERSION 3.10.2 )
project( vulkan_tutorial VERSION 0.1.0 )

option( BUILD_SHARED_LIBS "Build libraries as shared or not." TRUE )
set( CMAKE_CXX_STANDARD 17 )

# Add the local `cmake` directory to the CMake Module path.
list( INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake" )

include( compiler_flags )

# Package Dependencies
# - Next steps: learn from KWIVER, create different "depends" cmake modules
#   to encapsulate package specific include behavior instead of everything just
#   being right here, which is really messy.
find_package( glm REQUIRED )
find_package( glfw3 REQUIRED )
find_package( Vulkan REQUIRED )
include_directories( ${Vulkan_INCLUDE_DIRS} )

# Introspect the location of the install tree's layer path
file( TO_CMAKE_PATH "${Vulkan_LIBRARY}" VK_LAYER_PATH )
get_filename_component( VK_LAYER_PATH "${VK_LAYER_PATH}" DIRECTORY )
get_filename_component( VK_LAYER_PATH "${VK_LAYER_PATH}" DIRECTORY )
string( APPEND VK_LAYER_PATH "/etc/vulkan/explicit_layer.d" )
message( STATUS "VK_LAYER_PATH: ${VK_LAYER_PATH}" )

# Include project header files within sub-directories.
# - Assumes headers will be installed
include_directories( SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}" )

# Descend into the source
add_subdirectory( myengine )
add_subdirectory( tools )
