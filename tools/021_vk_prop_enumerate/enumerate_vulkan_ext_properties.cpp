/**
 * Little tool to dump out available vulkan instance extension properties.
 */
#include <cassert>
#include <iostream>

#include <vulkan/vulkan.hpp>

#include <myengine/logging.hpp>
#include <myengine/glfw.hpp>
#include <myengine/vulkan/devices.hpp>
#include <myengine/vulkan/instance.hpp>


int
main()
{
  //////////////////////////////////////////////////////////////////////////////
  // Instance Extension Properties
  //
  LOG_INFO( "Available Vulkan instance extensions (spec version):" );
  auto ext_prop_vec = myengine::vulkan::get_instance_extension_properties();
  for( auto const &ext : ext_prop_vec )
  {
    LOG_INFO( "-- " << ext.extensionName << " (" << ext.specVersion << ")" );
  }
  LOG_INFO( "" );

  //////////////////////////////////////////////////////////////////////////////
  // Instance Layer Properties
  //
  LOG_INFO( "Available Vulkan instance layers:" );
  auto layer_prop_vec = myengine::vulkan::get_instance_layer_properties();
  for( auto const &layer : layer_prop_vec )
  {
    LOG_INFO( "-- " << layer.layerName << ": " << layer.description );
  }
  LOG_INFO( "" );

  //////////////////////////////////////////////////////////////////////////////
  // Physical Devices
  //
  LOG_INFO( "Available physical devices:" );
  // need to create a simple instance?
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = "tempApp";
  app_info.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
  app_info.pEngineName = "tempEngine";
  app_info.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
  app_info.apiVersion = VK_API_VERSION_1_1;
  VkInstanceCreateInfo create_info = {};  // note: initialization is important.
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;
  // use what's found above?
  create_info.enabledExtensionCount = 0;
  create_info.enabledLayerCount = 0;
  VkInstance vkinstance;
  VkResult res = vkCreateInstance( &create_info, nullptr, &vkinstance );
  if( res != VK_SUCCESS )
  {
    LOG_ERROR( "Failed to create Vulkan instance for physical device query: " << res );
    return 1;
  }

  auto pdevice_vec = myengine::vulkan::get_physical_devices( vkinstance );
  for( auto const &device : pdevice_vec )
  {
    VkPhysicalDeviceProperties pd_prop = {};
    vkGetPhysicalDeviceProperties( device, &pd_prop );
    LOG_INFO( "-- ID:" << pd_prop.deviceID << " :: " << pd_prop.deviceName );
    LOG_INFO( "     Properties:" );
    LOG_INFO( "       API version supported: " << VK_VERSION_MAJOR( pd_prop.apiVersion )
                                               << "." << VK_VERSION_MINOR( pd_prop.apiVersion )
                                               << "." << VK_VERSION_PATCH( pd_prop.apiVersion ) );

    // The support for optional features like texture compression,
    // 64 bit floats and multi viewport rendering (useful for VR) can be
    // queried using `vkGetPhysicalDeviceFeatures`.

    auto queue_props_vec( myengine::vulkan::get_device_queue_family_properties( device ) );
    LOG_INFO( "    Queue Families" );
    for( auto const &queue_props : queue_props_vec )
    {
      vk::to_string( vk::QueueFlagBits::eGraphics );
      LOG_INFO( "      " << queue_props.queueCount << "x w/ Flags: " << queue_props.queueFlags );
    }
  }

  return 0;
}
