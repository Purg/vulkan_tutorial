#include "glfw.hpp"

#include <GLFW/glfw3.h>


namespace myengine::glfw
{

std::vector<char const *>
glfw_get_required_vulkan_extensions()
{
  uint32_t count = 0;
  char const **name_array;
  name_array = glfwGetRequiredInstanceExtensions( &count );
  return std::vector<char const *>( name_array, name_array + count );
}

}
