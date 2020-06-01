#include "devices.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

#include <vulkan/vulkan.hpp>


namespace myengine::vulkan
{

std::vector<VkPhysicalDevice>
get_physical_devices( VkInstance const &instance, physical_device_filter_t const &filter )
{
  uint32_t device_count = 0;
  vkEnumeratePhysicalDevices( instance, &device_count, nullptr );
  std::vector<VkPhysicalDevice> device_vec( device_count );
  VkResult res = vkEnumeratePhysicalDevices( instance, &device_count, device_vec.data() );
  if( res != VK_SUCCESS )
  {
    std::stringstream ss;
    ss << "Failed to enumerate any physical devices: "
       << vk::to_string( static_cast<vk::Result>(res) );
    throw std::runtime_error( ss.str() );
  }
  if( filter != nullptr )
  {
    std::vector<VkPhysicalDevice> filtered_vec( device_count );
    for( auto device : device_vec )
    {
      if( filter( device ) )
      {
        filtered_vec.push_back( device );
      }
    }
    device_vec = filtered_vec;
  }
  return device_vec;
}

}
