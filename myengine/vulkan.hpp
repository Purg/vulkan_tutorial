#ifndef MYENGINE_VULKAN_HPP
#define MYENGINE_VULKAN_HPP

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>


namespace myengine::vulkan
{

/// Type for a function that makes some decision on an input physical device.
typedef std::function<bool( VkPhysicalDevice const & )> physical_device_filter_t;


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


/// Get an enumeration of available physical devices.
/**
 * @param instance Vulkan instance to enumerate devices with respect to.
 * @param filter Optional function to levy a filter criterion on enumerated devices.
 *   Only those devices that return a true return from this function will be continued
 *   into the output of this function.
 *   TODO: Make change this to a score value filter instead of a boolean filter?
 *         This doesn't even need to be here, I just wanted to fuck with function pointers.
 *
 * @raises std::runtime_error
 *   Failed to enumerate physical devices via Vulkan API.
 *
 * @returns Vector of physical device handles, that have also optionally passed the give
 *   `filter` function with a `true` value.s
 */
std::vector<VkPhysicalDevice>
get_physical_devices( VkInstance const &instance,
                      physical_device_filter_t const &filter = nullptr );


/// Get an enumeration of queue family properties for the given device,
/**
 * @param device Physical device handle to query queue families from.
 * @return Vector of `VkQueueFamilyProperties` structs. The length of this vector is the
 *   returned value to `pQueueFamilyPropertyCount` when calling
 *   `vkGetPhysicalDeviceQueueFamilyProperties`.
 */
std::vector<VkQueueFamilyProperties>
get_device_queue_family_properties( VkPhysicalDevice const &device );

}

#endif //MYENGINE_VULKAN_HPP
