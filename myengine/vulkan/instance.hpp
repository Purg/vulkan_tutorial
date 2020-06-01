#ifndef MYENGINE_VULKAN_INSTANCE_H
#define MYENGINE_VULKAN_INSTANCE_H

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>


namespace myengine::vulkan
{

/**
 * Get the available Vulkan instance extension properties.
 *
 * @returns
 *   A vector of structs for extension properties currently available for Vulkan
 *   instance creation.
 */
std::vector<VkExtensionProperties>
get_instance_extension_properties();

/**
 * Get the available Vulkan instance layer properties.
 *
 * @returns
 *   A vector of structs for layer properties currently available for Vulkan
 *   instance creation.
 */
std::vector<VkLayerProperties>
get_instance_layer_properties();

/**
 * Check the requested Vulkan instance extension names against the available
 * extensions reported by the Vulkan SDK.
 *
 * @param [in] requested_exts
 *   Vector of string extension names to compare against available extensions.
 * @return True of all names in the input vector match some extension reported
 *   as available by Vulkan, false otherwise.
 *
 * @sa get_instance_extension_properties
 */
bool
check_instance_extension_support( std::vector<char const *> const &requested_exts );

/**
 * Check the requested Vulkan instance layer names against the available layers
 * reported by the Vulkan SDK.
 *
 * If additional validation layers are required, check your VK_LAYER_PATH
 * environment variable.
 *
 * @param [in] requested_layers
 *   Vector of string layer names to compare against available layers.
 * @return True of all names in the input vector match some layer reported
 *   as available by Vulkan, false otherwise.
 *
 * @sa get_instance_layer_properties
 */
bool
check_instance_layer_support( std::vector<char const *> const &requested_layers );

}

#endif //MYENGINE_VULKAN_INSTANCE_H
