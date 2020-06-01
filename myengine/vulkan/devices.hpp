#ifndef MYENGINE_VULKAN_DEVICES_H
#define MYENGINE_VULKAN_DEVICES_H

#include <functional>
#include <vector>

#include <vulkan/vulkan.h>


namespace myengine::vulkan
{

/// Type for a function that makes some decision on an input physical device.
typedef std::function<bool( VkPhysicalDevice const & )> physical_device_filter_t;


/// Get an enumeration of available physical devices.
/**
 * @param instance Vulkan instance to enumerate devices with respect to.
 * @param filter Optional function to levy a filter criterion on enumerated devices.
 *   Only those devices that return a true return from this function will be continued
 *   into the output of this function.
 *   TODO: Make change this to a score value filter instead of a boolean filter?
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

}

#endif //MYENGINE_VULKAN_DEVICES_H
