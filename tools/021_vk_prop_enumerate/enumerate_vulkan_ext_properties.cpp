/**
 * Little tool to dump out available vulkan instance extension properties.
 */

#include <vulkan/vulkan.h>

#include <myengine/logging.h>
#include <myengine/vulkan/instance.h>


int
main()
{
  uint32_t count;
  VkResult vk_res;

  //////////////////////////////////////////////////////////////////////////////
  // Instance Extension Properties
  //
  LOG_INFO("Available Vulkan instance extensions (spec version):");
  auto ext_prop_vec = myengine::vulkan::get_instance_extension_properties();
  for( auto const &ext : ext_prop_vec )
  {
    LOG_INFO("-- " << ext.extensionName << " (" << ext.specVersion << ")");
  }

  //////////////////////////////////////////////////////////////////////////////
  // Instance Layer Properties
  //
  LOG_INFO("Available Vulkan instance layers:");
  auto layer_prop_vec = myengine::vulkan::get_instance_layer_properties();
  for( auto const &layer : layer_prop_vec )
  {
    LOG_INFO("-- " << layer.layerName << ": " << layer.description);
  }

  return 0;
}
