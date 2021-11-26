#include "vulkan.h"

#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vulkan/vulkan.hpp>


namespace myengine::vulkan
{

std::vector<VkExtensionProperties>
get_instance_extension_properties()
{
  uint32_t count;
  vkEnumerateInstanceExtensionProperties( nullptr, &count, nullptr );
  std::vector<VkExtensionProperties> prop_vec( count );
  vkEnumerateInstanceExtensionProperties( nullptr, &count, prop_vec.data() );
  return prop_vec;
}


std::vector<VkLayerProperties>
get_instance_layer_properties()
{
  uint32_t count;
  vkEnumerateInstanceLayerProperties( &count, nullptr );
  std::vector<VkLayerProperties> layer_vec( count );
  vkEnumerateInstanceLayerProperties( &count, layer_vec.data() );
  return layer_vec;
}


bool
check_instance_extension_support( std::vector<char const *> const &requested_exts )
{
  // move construction i think?
  auto prop_vec( get_instance_extension_properties() );
  bool ext_found;
  for( auto const &req_name : requested_exts )
  {
    ext_found = false;
    for( auto const &prop : prop_vec )
    {
      if( strcmp( req_name, prop.extensionName ) == 0 )
      {
        ext_found = true;
        break;
      }
    }
    if( !ext_found )
    {
      return false;
    }
  }
  return true;
}


bool
check_instance_layer_support( std::vector<const char *> const &requested_layers )
{
  // move construction i think?
  auto layer_vec( get_instance_layer_properties() );
  bool layer_found;
  for( auto const &req_layer : requested_layers )
  {
    layer_found = false;
    for( auto const &prop : layer_vec )
    {
      if( strcmp( req_layer, prop.layerName ) == 0 )
      {
        layer_found = true;
        break;
      }
    }
    if( !layer_found )
    {
      return false;
    }
  }
  return true;
}


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
    std::vector<VkPhysicalDevice> filtered_vec;
    filtered_vec.reserve( device_count );
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


std::vector<VkQueueFamilyProperties>
get_device_queue_family_properties( VkPhysicalDevice const &device )
{
  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties( device, &count, nullptr );
  std::vector<VkQueueFamilyProperties> vec( count );
  vkGetPhysicalDeviceQueueFamilyProperties( device, &count, vec.data() );
  return vec;
}

}
