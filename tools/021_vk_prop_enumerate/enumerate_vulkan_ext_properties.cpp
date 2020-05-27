/**
 * Little tool to dump out available vulkan instance extension properties.
 */
#include <vector>

#include <vulkan/vulkan.h>

#include <myengine/logging.h>


int
main()
{
  uint32_t count;
  VkResult vk_res;

  //////////////////////////////////////////////////////////////////////////////
  // Instance Extension Properties
  //
  // Query for the number of global instance properties.
  vk_res = vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);
  if( vk_res != VK_SUCCESS )
  {
    LOG_ERROR("FAILED to query for number of vulkan instance extension "
              "properties. (error: " << vk_res << ")");
    return vk_res;
  }
  // Query for properties array
  std::vector<VkExtensionProperties> extensions(count);
  vk_res = vkEnumerateInstanceExtensionProperties(nullptr, &count,
                                                  extensions.data());
  if( vk_res != VK_SUCCESS )
  {
    LOG_ERROR("FAILED to query for array of vulkan instance extension "
              "properties. (error: " << vk_res << ")");
    return vk_res;
  }
  // Report available instance extensions
  LOG_INFO("Available Vulkan instance extensions (spec version):");
  for( auto const &ext : extensions )
  {
    LOG_INFO("-- " << ext.extensionName << " (" << ext.specVersion << ")");
  }

  //////////////////////////////////////////////////////////////////////////////
  // Instance Layer Properties
  //
  // Query for the number of layer properties
  vk_res = vkEnumerateInstanceLayerProperties(&count, nullptr);
  if( vk_res != VK_SUCCESS )
  {
    LOG_ERROR("Failed to query for the number of vulkan instance layer "
              "properties (error: " << vk_res << ")");
    return vk_res;
  }
  // Query for properties array
  std::vector<VkLayerProperties> layers(count);
  vk_res = vkEnumerateInstanceLayerProperties(&count, layers.data());
  // report
  LOG_INFO("Available Vulkan instance layers:");
  for( auto const &layer : layers )
  {
    LOG_INFO("-- " << layer.layerName << ": " << layer.description);
  }

  return 0;
}
