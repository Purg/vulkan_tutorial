#ifndef GLFW_H
#define GLFW_H

#include <vector>

#include <myengine/myengine_export.h>

namespace myengine::glfw {

/**
 * Get the required vulkan extensions by name from GLFW's API.
 *
 * @return Vector of required extensions by string name.
 */
[[nodiscard]] std::vector< char const* >
MYENGINE_EXPORT
glfw_get_required_vulkan_extensions();

} // namespace myengine::glfw

#endif //GLFW_H
