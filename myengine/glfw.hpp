#ifndef GLFW_H
#define GLFW_H

#include <vector>


namespace myengine::glfw
{

/**
 * Get the required vulkan extensions by name from GLFW's API.
 *
 * @return Vector of required extensions by string name.
 */
[[nodiscard]] std::vector<char const *>
glfw_get_required_vulkan_extensions();

}

#endif //GLFW_H
