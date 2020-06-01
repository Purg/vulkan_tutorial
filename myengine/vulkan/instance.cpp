//
// Created by paul on 5/27/20.
//

#include <cstring>

#include "instance.hpp"


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

}
