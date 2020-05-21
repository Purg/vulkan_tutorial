/**
 * Little tool to dump out available vulkan instance extension properties.
 */
#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>


int
main()
{
  uint32_t ext_count;
  // Query for the number of global instance properties.
  VkResult res =
      vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
  if( res != VK_SUCCESS )
  {
    std::cerr << "FAILED to query for number of vulkan instance extension "
                 "properties. (error: " << res << ")" << std::endl;
    return res;
  }
  // Query for properties array
  std::vector<VkExtensionProperties> extensions(ext_count);
  res = vkEnumerateInstanceExtensionProperties(nullptr, &ext_count,
                                               extensions.data());
  if( res != VK_SUCCESS )
  {
    std::cerr << "FAILED to query for array of vulkan instance extension "
                 "properties. (error: " << res << ")" << std::endl;
    return res;
  }

  std::cout << "Available Vulkan instance extensions (spec version):\n";
  for( auto const &ext : extensions )
  {
    std::cout << "\t" << ext.extensionName << " (" << ext.specVersion << ")\n";
  }

  return 0;
}
