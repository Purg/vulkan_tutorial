#include <algorithm>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// This defines required or otherwise include Vulkan first.
// Some Vulkan definitions are required to activate Vulkan-specific
// functionality.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <myengine/glfw.h>
#include <myengine/logging.h>
#include <myengine/vulkan.h>

#define VK_EXT_debug_utils_NAME "VK_EXT_debug_utils"

struct QueueFamilyIndices
{
  std::optional< uint32_t > graphicsFamily;
  std::optional< uint32_t > presentFamily;

  /// If this struct instance has all its optional fields filled.
  [[nodiscard]]

  bool
  is_complete() const
  {
    return ( graphicsFamily.has_value() && presentFamily.has_value() );
  }
};

struct SwapChainSupportInfo
{
  VkSurfaceCapabilitiesKHR capabilities;
  std::vector< VkSurfaceFormatKHR > formats;
  std::vector< VkPresentModeKHR > present_modes;

  /// Default constructor
  SwapChainSupportInfo()
    : capabilities{},
      formats(),
      present_modes()
  {}

  /// Copy Constructor
  SwapChainSupportInfo( SwapChainSupportInfo const& other ) = default;

  /// Move constructor
  SwapChainSupportInfo( SwapChainSupportInfo&& other ) = default;
};

/**
 * Access the global static list of validation layers to be used.
 *
 * This is a singleton/construct-on-first-use idiom.
 *
 * @return The vector of string validation layer names.
 */
std::vector< char const* > const&
STATIC_INSTANCE_VALIDATION_LAYERS()
{
  static std::vector< char const* > const v =
  { "VK_LAYER_KHRONOS_validation" };
  return v;
}

/// Singleton vector of device extensions that our application uses.
std::vector< char const* > const&
STATIC_DEVICE_EXTENSIONS()
{
  static std::vector< char const* > const v =
  { VK_KHR_SWAPCHAIN_EXTENSION_NAME, };
  return v;
}

/**
 * Callback linkup with debug logging.
 *
 * The use of this callback at all is intended to be conditional on !NDEBUG.
 * With that, I guess "verbose" messages are on the table to be output?
 *
 * NOTE: The *real* way to go here is probably for this whole thing to take in
 * parametrization for multiple levels of verbosity for finer grain control
 * (e.g. want info+ logging, but just for validation types, etc.).
 */
VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_messenger_logging_hook(
  VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
  VkDebugUtilsMessageTypeFlagsEXT msg_type,
  VkDebugUtilsMessengerCallbackDataEXT const* p_callback_data,
  void* p_user_data )
{
  // msg_severity==INFO is actually pretty verbose. Not sure if INFO or VERBOSE
  // is the "Debug" level
  // but INFO seems to so far be the more verbose one. Skipping that one for
  // the moment until
  // something goes sideways.
  if( msg_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT )
  {
    return VK_FALSE;
  }

  auto s_severity =
    vk::to_string( (vk::DebugUtilsMessageSeverityFlagBitsEXT) msg_severity );
  auto s_type =
    vk::to_string( (vk::DebugUtilsMessageTypeFlagBitsEXT) msg_type );
  std::cerr     << "Khronos"
                << "[" << s_severity << "]"
                << "[type::" << s_type << "] "
                << p_callback_data->pMessage << std::endl;
  return VK_FALSE;
}

/**
 * Fill in a given create-info struct for a debug messenger given this
 * tutorial/app context.
 *
 * @param [in,out] create_info Info struct to update values of.
 */
void
vk_debug_messenger_create_info_fill(
  VkDebugUtilsMessengerCreateInfoEXT& create_info )
{
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity =
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  // maybe remove this later if it's excessive?
  create_info.messageSeverity |=
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  create_info.pfnUserCallback = vk_debug_messenger_logging_hook;
}

/**
 * Create a new debug messenger handle for the given vulkan instance.
 *
 * This registers `vk_debug_messenger_logging_hook` for stuff.
 *
 * @param [in] instance
 *   Vulkan instance handle to create the debug messenger against.
 * @returns Optionally created debug messenger opaque handle. This may be null
 *   if the appropriate extension function fails to dynamically load.
 *
 * @sa `create_vulkan_instance` for note on `[[ nodiscard ]]` attribute.
 */
[[nodiscard]] VkDebugUtilsMessengerEXT
vk_createDebugMessenger( VkInstance const& instance )
{
  // As long as I go in order and don't skip anything, gcc is OK...
  VkDebugUtilsMessengerCreateInfoEXT create_info = {};
  vk_debug_messenger_create_info_fill( create_info );

  auto create_func = (PFN_vkCreateDebugUtilsMessengerEXT)
                     vkGetInstanceProcAddr( instance,
                                            "vkCreateDebugUtilsMessengerEXT" );
  VkResult create_result;
  VkDebugUtilsMessengerEXT debug_messenger_handle = nullptr;
  if( create_func == nullptr )
  {
    create_result = VK_ERROR_EXTENSION_NOT_PRESENT;
  }
  else
  {
    create_result = create_func( instance, &create_info, nullptr,
                                 &debug_messenger_handle );
  }
  if( create_result != VK_SUCCESS )
  {
    std::stringstream ss;
    ss  << "Failed to create debug messenger! "
        << vk::to_string( static_cast< vk::Result >( create_result ) );
    throw std::runtime_error( ss.str() );
  }
  return debug_messenger_handle;
}

/**
 * Destroy a vulkan debug messenger given its handle and its associated vulkan
 * instance handle.
 *
 * @param [in] instance
 *   The associated Vulkan instance.
 * @param [in] debugMessenger
 *   The messenger instance handle to destroy.
 */
void
vk_destroyDebugMessenger( VkInstance const& instance,
                          VkDebugUtilsMessengerEXT const& messenger )
{
  auto destroy_func =
    (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(
      instance, "vkDestroyDebugUtilsMessengerEXT" );
  if( messenger != nullptr && destroy_func != nullptr )
  {
    destroy_func( instance, messenger, nullptr );
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Application Functions

/// App-specific validation layer addition logic
void
populate_app_validation_layers( std::vector< char const* >& validation_layers )
{
  // We only want to add validation layers if we're flagged for debug.
#ifndef NDEBUG
  auto p_validation_layers = &STATIC_INSTANCE_VALIDATION_LAYERS();
  validation_layers.insert( validation_layers.end(),
                            p_validation_layers->cbegin(),
                            p_validation_layers->cend() );
#endif
}

/**
 * Instantiate the Vulkan Instance
 *
 * This could probably be made into a utility func in the engine lib.
 * Could take parameters for the app_info stuff, as well as extensions and
 * validation layers.
 *
 * TIL about the [[nodiscard]] attribute. This is for attributing a member
 * function whose critical value is its return, encouraging the compiler to
 * warn with such an attributed method is invoked *without* its return value
 * captured.
 * This is noted for "since c++17" but the appropriate warning is showing up
 * with "gnu++11" as well.
 *
 * NOTES:
 *   - Given the vulkan documentation for `VkApplicationInfo`, does that mean
 * that `pEngineName`
 *     should be "constant" if generated by "myengine"? Does this not matter if
 * we're defining it
 *     here? Seems like this would be defined by the library providing the
 * wrapping around vulkan
 *     while the user utilizing that library provides the application
 * name/version.
 *
 * @param app_name String name of the application (null-terminated UTF-8).
 * @param app_version Version uint of the application. See `VK_MAKE_VERSION`.
 *
 * @throws std::runtime_error
 *   Requested instance extension or validation layer not currently supported.
 *
 * @sa VK_MAKE_VERSION
 */
[[nodiscard]] VkInstance
create_vulkan_instance( char const* app_name, uint32_t app_version,
                        uint32_t vk_api_version = VK_API_VERSION_1_1 )
{
  // out-of-order designated initializer apparently unimplemented in g++, so
  // not doing that here... sad...
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pApplicationName = app_name;
  app_info.applicationVersion = app_version;
  app_info.pEngineName = app_name;
  app_info.engineVersion = app_version;
  // Maybe derive from external/CMake setting?
  app_info.apiVersion = vk_api_version;

  // TODO: Could probably function up the create_info and
  VkInstanceCreateInfo create_info = {};
  create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  create_info.pApplicationInfo = &app_info;

  std::vector< char const* > inst_extensions;
  std::vector< char const* > inst_validation_layers;

  // Register "next" pointer to debug messenger if in debug mode /////////////
  // This is to allow debug messenger to activate for instance
  // creation/destruction since the "main" debug messenger is created *after*
  // instance creation.
#ifndef NDEBUG
  LOG_DEBUG( "Creating debug messenger specifically for the Vulkan instance." );
  VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {};
  vk_debug_messenger_create_info_fill( debug_create_info );
  // linked-list insert at the head of the pNext chain.
  debug_create_info.pNext = create_info.pNext;
  create_info.pNext = &debug_create_info;
#endif

  // Request extensions //////////////////////////////////////////////////////
  // Enable the global extensions requested by the windowing system we're
  // using (GLFW).
  // NOTE: This part would get swapped out if moving to SDL.
  for( auto const& ext_name :
       myengine::glfw::glfw_get_required_vulkan_extensions() )
  {
    inst_extensions.push_back( ext_name );
  }
#ifndef NDEBUG
  inst_extensions.push_back( VK_EXT_debug_utils_NAME );
#endif

  // Request validation layers ///////////////////////////////////////////////
  // NDEBUG logic bundled within function
  populate_app_validation_layers( inst_validation_layers );

  // Enumerating/Checking requested extensions ///////////////////////////////
  LOG_INFO( "Requesting Vulkan instance extensions:" );
  for( auto const& ext_name : inst_extensions )
  {
    LOG_INFO( ".. '" << ext_name << "'" );
  }
  if( inst_extensions.empty() )
  {
    LOG_INFO( "(None)" );
  }
  if( !myengine::vulkan::check_instance_extension_support( inst_extensions ) )
  {
    throw std::runtime_error( "One or more instance extensions not reported "
                              "as available." );
  }
  create_info.enabledExtensionCount = inst_extensions.size();
  create_info.ppEnabledExtensionNames = inst_extensions.data();

  // Enumerating/Checking requested validation layers ////////////////////////
  LOG_INFO( "Requesting Vulkan instance validation layers:" );
  for( auto const& layer_name : inst_validation_layers )
  {
    LOG_INFO( ".. '" << layer_name << "'" );
  }
  if( inst_validation_layers.empty() )
  {
    LOG_INFO( "(None)" );
  }
  if( !myengine::vulkan::check_instance_layer_support( inst_validation_layers ) )
  {
    throw std::runtime_error( "One or more instance validation layers not "
                              "reported as available." );
  }
  create_info.enabledLayerCount = inst_validation_layers.size();
  create_info.ppEnabledLayerNames = inst_validation_layers.data();

  // Actually create the instance...
  VkInstance instance;
  VkResult result = vkCreateInstance( &create_info, nullptr,
                                      &instance );
  if( result != VK_SUCCESS )
  {
    std::stringstream ss;
    ss  << "Failed to initialize Vulkan instance with error code " << result
        << " (" << vk::to_string( static_cast< vk::Result >( result ) ) << ")";
    throw std::runtime_error( ss.str() );
  }
  return instance;
}

/**
 * Create a surface through GLFW, returning an opaque handle to the created
 * surface.
 *
 * @param [in] instance Vulkan instance to use for surface creation.
 * @param [in] window GLFW window
 *
 * @throws std::logic_error GLFW failed to think vulkan is supported.
 * @throws std::runtime_error Failed to create the VkSurfaceKHR instance.
 *
 * @return Opaque handle to the created surface.
 */
[[nodiscard]] VkSurfaceKHR
create_vulkan_surface( VkInstance const& instance, GLFWwindow* window )
{
  if( glfwVulkanSupported() != GLFW_TRUE )
  {
    throw std::logic_error( "GLFW did not indicate vulkan is being supported" );
  }

  VkSurfaceKHR surface;
  VkResult res =
    glfwCreateWindowSurface( instance, window, nullptr, &surface );
  if( res != VK_SUCCESS )
  {
    std::stringstream ss;
    ss << "Failed to create Vulkan surface through GLFW: " <<
      vk::to_string( (vk::Result) res );
    throw std::runtime_error( ss.str() );
  }
  return surface;
}

/**
 * Update the given `QueueFamilyIndices` structure reference with the device's
 * supported queue families.
 *
 * This currently assumes that there is only one queue per tracked index type,
 * or that the first
 * queue family found per tracked index type is acceptable.
 *
 * We currently prefer setting the graphics and presentation indices to a queue
 * family that
 * supports both.
 *
 * TODO: Candidate for being a library utility function?
 *
 * @param [in] device Physical device to query the queue properties of.
 * @param [out] indices Structure to set queue indices to, overwriting any
 * previous values.
 *
 * @throws std::runtime_error Failed to query for KHR surface support.
 */
void
find_queue_families( VkPhysicalDevice const& device,
                     VkSurfaceKHR const& surface,
                     QueueFamilyIndices& indices )
{
  // Tutorial: you could add logic to explicitly prefer a physical device that
  // supports drawing
  // and presentation in the same queue for improved performance.
  // - Sure, why not.
  auto queue_fam_props_vec =
    myengine::vulkan::get_device_queue_family_properties( device );
  bool graphics_support;
  VkBool32 present_support = false;
  VkResult vk_res;
  int i = 0;
  for( auto const& queue_fam_props : queue_fam_props_vec )
  {
    graphics_support = queue_fam_props.queueFlags & VK_QUEUE_GRAPHICS_BIT;
    vk_res = vkGetPhysicalDeviceSurfaceSupportKHR( device, i, surface,
                                                   &present_support );
    if( vk_res != VK_SUCCESS )
    {
      throw std::runtime_error( vk::to_string( (vk::Result) vk_res ) );
    }
    // If both indices aren't already set to the same value, and both graphics
    // and surface
    // presentation are supported by the current index, set it to both indices.
    if( ( !indices.is_complete() ||
          ( indices.graphicsFamily.value() !=
            indices.presentFamily.value() ) ) &&
        graphics_support && present_support )
    {
      LOG_DEBUG( "Found queue_fam idx " << i <<
                 " supports both graphics and surface presentation!" );
      indices.graphicsFamily = indices.presentFamily = i;
    }
    else if( !indices.graphicsFamily.has_value() && graphics_support )
    {
      LOG_DEBUG( "Found queue fam idx " << i << " supports graphics." );
      indices.graphicsFamily = i;
    }
    else if( !indices.presentFamily.has_value() &&
             ( present_support == VK_TRUE ) )
    {
      LOG_DEBUG(
        "Found queue fam idx " << i << " supports surface presentation." );
      indices.presentFamily = i;
    }
    // Increment to the next index value.
    ++i;
  }
}

/**
 * Check if the given physical device supports application required extensions
 *
 * See `STATIC_DEVICE_EXTENSIONS` function for the vector of extensions
 * required by this
 * application.
 *
 * @param device Physical device to check.
 * @return True if all extension
 *
 * @sa STATIC_DEVICE_EXTENSIONS
 */
[[nodiscard]]

bool
check_device_extensions_support( VkPhysicalDevice const& device,
                                 std::vector< char const* > const& device_extension_names )
{
  uint32_t extension_count;
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count,
                                        nullptr );

  std::vector< VkExtensionProperties > available_extensions( extension_count );
  vkEnumerateDeviceExtensionProperties( device, nullptr, &extension_count,
                                        available_extensions.data() );

  // Shallow copy required extension names into a set, removing reportedly
  // available extensions from
  // this. If all required extensions are supported, the set will end up empty.
  std::set< std::string > required_extensions( device_extension_names.begin(),
                                               device_extension_names.end() );
  for( auto const& ext : available_extensions )
  {
    required_extensions.erase( ext.extensionName );
  }
  if( not required_extensions.empty() )
  {
    VkPhysicalDeviceProperties p = {};
    vkGetPhysicalDeviceProperties( device, &p );
    LOG_DEBUG(
      "Not all extensions supported for device '" << p.deviceName << "'!" );
    for( auto const& n : required_extensions )
    {
      LOG_DEBUG( "\t- " << n );
    }
  }

  return required_extensions.empty();
}

/**
 * Query physical device swap-chain support information.
 *
 * @param device Physical device to query.
 * @param surface
 */
SwapChainSupportInfo
query_swapchain_support( VkPhysicalDevice const& device,
                         VkSurfaceKHR const& surface )
{
  SwapChainSupportInfo info;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface,
                                             &info.capabilities );

  uint32_t count;
  vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &count, nullptr );
  if( count != 0 )
  {
    info.formats.resize( count );
    vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &count,
                                          info.formats.data() );
  }

  vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &count,
                                             nullptr );
  if( count != 0 )
  {
    info.present_modes.resize( count );
    vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &count,
                                               info.present_modes.data() );
  }

  return info;
}

/**
 * App-specific physical device selection criterion.
 *
 * Binary selection criterion on those aspects that are 100% required.
 *
 * @param device Physical device to check suitability of.
 * @param surface Consider this surface in suitability checks.
 * @param device_extension_names Required device extension names. This is empty
 * by default.
 *
 * @return Input device is suitable.
 */
[[nodiscard]] bool
is_suitable_device( VkPhysicalDevice const& device,
                    VkSurfaceKHR const& surface,
                    std::vector< char const* > const& device_extension_names = {} )
{
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties( device, &props );
  LOG_DEBUG(
    "Considering device (" << props.deviceID << ") '" << props.deviceName <<
      "'" );

  // See
  // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Base-device-suitability-checks
  // for alternative examples.

  // A device is suitable if it supports queue family indices that we care
  // about.
  QueueFamilyIndices indices = {};
  find_queue_families( device, surface, indices );

  // A device is suitable if it supports the device extensions this application
  // desires.
  // - parameterize vector of layers?
  bool extensions_supported = check_device_extensions_support( device,
                                                               device_extension_names );

  bool swapchain_adequate = false;
  if( extensions_supported )
  {
    SwapChainSupportInfo sc_info = query_swapchain_support( device, surface );
    swapchain_adequate =
      ( !sc_info.formats.empty() && !sc_info.present_modes.empty() );
  }

  return indices.is_complete() && extensions_supported && swapchain_adequate;
}

/**
 * App-specific scoring of a physical device for use.
 *
 * @param device Physical device to score.
 * @param surface Surface to which the device will display to to consider for
 * scoring.
 *
 * @return Score for the physical device.
 */
uint32_t
score_physical_device( VkPhysicalDevice const& device )
{
  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties( device, &props );
  // Not considering features yet.
  // VkPhysicalDeviceFeatures feats;
  // vkGetPhysicalDeviceFeatures( device, &feats );
  LOG_DEBUG(
    "Scoring device (" << props.deviceID << ") '" << props.deviceName << "'" );

  uint32_t score = 0;

  // We are going to prefer (greater-than) DISCRETE > VIRTUAL > INTEGRATED >
  // CPU
  // VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
  switch( props.deviceType )
  {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      score |= 0b1000;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      score |= 0b0100;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      score |= 0b0010;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      score |= 0b0001;
      break;
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
    default:
      // Leave at 0.
      break;
  }

  return score;
}

/**
 * Decide which physical device is to be used.
 *
 * Copy return is probably OK and performant because a VkPhysicalDevice is just
 * an opaque handle
 * (pointer).
 *
 * @param instance Vulkan instance to base physical device selection from.
 * @param surface Surface to use in determining physical device to use.
 * @param device_extension_names Required device extensions that must be
 * supported. This is empty by default.
 *
 * @throws std::runtime_error No physical devices that pass the filter
 * criterion.
 * @return A singular physical device handle to be used.
 */
[[nodiscard]]

VkPhysicalDevice
pick_physical_device( VkInstance const& instance, VkSurfaceKHR const& surface,
                      std::vector< char const* > const& device_extension_names = {} )
{
  // Get available physical devices that pass initial hard selection criterion.
  LOG_DEBUG( "Getting suitable physical devices." );

  std::vector< VkPhysicalDevice > device_vec =
    myengine::vulkan::get_physical_devices(
      instance,
      [ & ]( VkPhysicalDevice const& device ) -> bool {
        return is_suitable_device( device, surface, device_extension_names );
      } );
  LOG_DEBUG(
    "Found " << device_vec.size() <<
      " physical devices after hard selection." );

  if( device_vec.empty() )
  {
    throw std::runtime_error( "Zero physical devices discovered post filter." );
  }

  if( device_vec.size() > 1 )
  {
    LOG_INFO( "Found more than one suitable physical device! " );
    // Sort available devices by score.
    std::stable_sort(
      device_vec.begin(), device_vec.end(),
      []( VkPhysicalDevice const& d1, VkPhysicalDevice const& d2 ) -> bool {
        return score_physical_device( d1 ) > score_physical_device( d2 );
      } );

    // Get the name for reporting.
    VkPhysicalDeviceProperties p = {};
    vkGetPhysicalDeviceProperties( device_vec[ 0 ], &p );
    LOG_INFO( "Using '" << p.deviceName << "' with the highest score." );
  }

  // This is a dumb initial pass. Something more should be done instead of just
  // selecting th
  // first one in the list.
  return device_vec[ 0 ];
}

/**
 * Create logical devices for this app.
 *
 * Create a logical device off of the given physical device, creating a queue
 *
 * @param [in] physical_device Physical physical_device from which the logical
 * physical_device
 *   should be created.
 * @param [in] qf_indices Queried queue family indices for the given device as
 * from
 *   `find_queue_families`.
 * @param [in] device_extension_names Vector of names of the device extensions
 * to
 *
 * @throws std::bad_optional_access If we request the index of a queue family
 * that the physical
 *    device does not support.
 * @throws std::runtime_error Failed to create the logical device.
 *
 * @returns Opaque handle to the newly created logical device.
 */
[[nodiscard]] VkDevice
create_logical_device( VkPhysicalDevice const& physical_device,
                       QueueFamilyIndices const& qf_indices,
                       std::vector< char const* > const& device_extension_names = {} )
{
  // Unique set of queue family indices to trigger queue creation on the
  // device.
  // Using a set here to automatically collapse shared index values among
  // recorded queue family
  // indices.
  std::set< uint32_t > q_fam_idx_set = {
    qf_indices.graphicsFamily.value(),
    qf_indices.presentFamily.value() };
  std::vector< VkDeviceQueueCreateInfo > q_create_info_vec;
  float q_priority = 1.f;

  for( uint32_t q_fam_idx : q_fam_idx_set )
  {
    VkDeviceQueueCreateInfo q_create_info = {};
    q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    q_create_info.queueFamilyIndex = q_fam_idx;
    // Generally only need a few concurrent queues as most.
    // Command buffers submitted to a single queue are executed ("started") in
    // order relative to each other. Commands submitted to different queues are
    // unordered relative to each other without explicit synchronization (see
    // `VkSemaphore`). Can only submit to a queue from one thread at a time (or
    // across multiple with "external" synchronization), while different
    // threads may submit to different queues simultaneously.
    //
    // So far I think the intent here is that it doesn't make sense to submit
    // to a graphics queue asynchronously (without more complicated
    // logic/synchronization at least).
    q_create_info.queueCount = 1;
    // A [0, 1] (inclusive) priority value for the queue to influence the
    // scheduling of command buffer execution. I'm interpreting that 1.0 is the
    // maximum priority.
    // - Just using the single float reference here is fine because
    //   `queueCount = 1` above.
    q_create_info.pQueuePriorities = &q_priority;
    // Something about setting system-wide priority level of the queue by
    // setting a `VkDeviceQueueGlobalPriorityCreateInfoEXT` to `pNext` (I see a
    // "realtime" option in there, maybe applicable for games?).
    q_create_info_vec.push_back( q_create_info );
  }

  // From Tutorial: Right now we don't need anything special, so we can simply
  // define it and leave everything to initialize to VK_FALSE.
  // We'll come back to this structure once we're about to start doing more
  // interesting things with Vulkan.
  VkPhysicalDeviceFeatures device_features = {};

  // Creation info struct for the logical device.
  VkDeviceCreateInfo d_create_info = {};
  d_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  d_create_info.queueCreateInfoCount = q_create_info_vec.size();
  d_create_info.pQueueCreateInfos = q_create_info_vec.data();

  // Device specific layers are currently deprecated as of at least vulkan 1.1.
  // The tutorial recommends setting them anyway to support older vulkan
  // implementations. OK FINE...
  std::vector< char const* > device_validation_layers;
  populate_app_validation_layers( device_validation_layers );
  d_create_info.enabledLayerCount = device_validation_layers.size();
  d_create_info.ppEnabledLayerNames = device_validation_layers.data();
  // Probably going to revisit this in a later tutorial chapter. Mentioned
  // "VK_KHR_swapchain".
  d_create_info.enabledExtensionCount = device_extension_names.size();
  d_create_info.ppEnabledExtensionNames = device_extension_names.data();
  // The features
  d_create_info.pEnabledFeatures = &device_features;

  VkDevice logical_device = VK_NULL_HANDLE;
  VkResult res = vkCreateDevice( physical_device, &d_create_info, nullptr,
                                 &logical_device );
  if( res != VK_SUCCESS )
  {
    std::stringstream ss;
    ss << "Failed to create logical device: " << vk::to_string(
      (vk::Result) res );
    throw std::runtime_error( ss.str() );
  }
  return logical_device;
}

/*******************************************************************************
 * Our home for the tutorial: a hello-world like class.
 *
 * Notes:
 * * Sounds like different things fill the roll of GLFW (SDL, GLUT, other
 *   platform specific things). Sounds like a point of modularity but I don't
 *   know enough about windowing or how its used here to know what's the common
 *   need / operation is.
 */
class HelloTriangleApp
{
public:
  char const* APP_NAME = "HelloTriangle";

  HelloTriangleApp()
    : HelloTriangleApp( 600, 800 )
  {}

  explicit HelloTriangleApp( uint32_t window_height, uint32_t window_width )
    : m_win_height( window_height ),
      m_win_width( window_width ),
      m_window( nullptr ),
      m_vk_instance_handle( VK_NULL_HANDLE ),
      m_vk_debug_messenger( VK_NULL_HANDLE ),
      m_vk_surface( VK_NULL_HANDLE ),
      m_vk_physical_device( VK_NULL_HANDLE ),
      m_vk_logical_device( VK_NULL_HANDLE ),
      m_vk_queue_graphics( VK_NULL_HANDLE ),
      m_vk_queue_present( VK_NULL_HANDLE )
  {}

  ~HelloTriangleApp() = default;

  void
  run()
  {
    m_window = initGlfwWindow();
    initVulkan( m_window );
    mainLoop();
    cleanUp();  // Call in destructor instead?
  }

private:
  // variables
  uint32_t m_win_height, m_win_width;
  GLFWwindow* m_window;
  // `VkInstance` *is* a pointer: to the empty `struct VkInstance_T` type.
  VkInstance m_vk_instance_handle;
  // Optional pointer to a debug messenger. May be null.
  VkDebugUtilsMessengerEXT m_vk_debug_messenger;
  // Rendering service for interfacing with windowing.
  VkSurfaceKHR m_vk_surface;
  // Opaque handle to the physical device to use.
  // Implicitly destroyed with the vulkan instance.
  VkPhysicalDevice m_vk_physical_device;
  // Opaque handle to the logical device for this app.
  VkDevice m_vk_logical_device;
  // Opaque handles for queues
  VkQueue m_vk_queue_graphics;
  VkQueue m_vk_queue_present;

private:
  // methods
  /// Tutorial: Initialize GLFW window instance to use.

  /**
   * Initialize GLFW and return a window handle. This should only be called
   * once.
   *
   * GLFW initialization is idempotent so calling this multiple times to create
   * multiple windows is
   * I guess a possibility.
   *
   * @throws std::runtime_error Failed to initialize GLFW or create a window.
   *
   * @returns New GLFW window instance handle.
   */
  [[nodiscard]]

  GLFWwindow*
  initGlfwWindow() const
  {
    int glfw_init_ret = glfwInit();
    if( glfw_init_ret != GLFW_TRUE )
    {
      std::stringstream ss;
      ss << "glfwInit() returned failure code " << glfw_init_ret;
      throw std::runtime_error( ss.str() );
    }

    // Prevent creation of OpenGL context (because we're using Vulkan...)
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    // Disable window resizing.
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    // To make full-screen, `monitor` will need to be specified and the
    // specified size will be the resolution, otherwise will be in windowed
    // mode.
    GLFWwindow* window = glfwCreateWindow(
      m_win_width, m_win_height, HelloTriangleApp::APP_NAME,
      nullptr, nullptr
      );
    if( !window )
    {
      throw std::runtime_error( "Failed to create a GLFW window." );
    }
    return window;
  }

  /// Tutorial: Initialize Vulkan components

  /**
   * @param [in] window The GLFW window instance handle to use for Vulkan
   * surface initialization.
   *
   * Post-condition: The following member variables should be defined with
   * valid handles after
   * successful execution of this method:
   *   - `m_vk_instance_handle`
   *   - `m_vk_surface`
   *   - `m_vk_physical_device`
   *   - `m_vk_logical_device`
   *   - `m_vk_queue_graphics`
   * The following is optionally defined if NDEBUG is NOT defined, otherwise it
   * is null:
   *   - `m_vk_debug_messenger`
   */
  void
  initVulkan( GLFWwindow* window )
  {
    LOG_DEBUG( "Creating application instance handle" );
    m_vk_instance_handle =
      create_vulkan_instance( APP_NAME, VK_MAKE_VERSION( 0, 1, 0 ) );
#ifndef NDEBUG
    LOG_DEBUG( "Creating debug messenger." );
    m_vk_debug_messenger =
      vk_createDebugMessenger( this->m_vk_instance_handle );
#endif
    m_vk_surface = create_vulkan_surface( m_vk_instance_handle, window );
    LOG_DEBUG( "Selecting physical device for use." );
    m_vk_physical_device = pick_physical_device( m_vk_instance_handle,
                                                 m_vk_surface,
                                                 STATIC_DEVICE_EXTENSIONS() );

    // technically a duplicate call, see `is_suitable_device`
    LOG_DEBUG(
      "Querying queue families on final physical device for logical device creation" );
    QueueFamilyIndices qf_indices = {};
    find_queue_families( m_vk_physical_device, m_vk_surface, qf_indices );
    m_vk_logical_device = create_logical_device( m_vk_physical_device,
                                                 qf_indices,
                                                 STATIC_DEVICE_EXTENSIONS() );

    LOG_DEBUG(
      "Let's grab the logical device's graphics/presentation queue(s)." );
    // Hardcoded `0` here "because we're only creating a single queue from this
    // family."
    // - I think this is tied to the `VkDeviceQueueCreateInfo.queueCount` value
    // which is currently
    //   set to `1` in `create_logical_device`.
    // - When the same queue family supports both graphics *and* surface
    // presentation, then only one
    //   queue is requested on the logical device. I presume this means that,
    // by specifying 0 to
    //   both of the following, we're storing the same queue handle for both
    // graphics and present?
    //   Is that OK? The tutorial seemed to basically recommend this by stating
    // that we could
    //   "prefer a physical device that supports drawing and presentation in
    // the same queue for
    //   improved performance".
    vkGetDeviceQueue( m_vk_logical_device,
                      qf_indices.graphicsFamily.value(), 0,
                      &m_vk_queue_graphics );
    vkGetDeviceQueue( m_vk_logical_device, qf_indices.presentFamily.value(), 0,
                      &m_vk_queue_present );
  }

  void
  mainLoop()
  {
    LOG_DEBUG( "Starting main loop..." );

    std::chrono::steady_clock::time_point
      last_t = std::chrono::steady_clock::now(),
      t = {};
    double avg_fps = 0, now_fps = 0;
    uint64_t avg_window = 120;

    while( !glfwWindowShouldClose( m_window ) )
    {
      // Probably take this out once we're actually doing anything?
      // Sleep optionally based on a remaining delta to "frame cap"?
      std::this_thread::sleep_for( std::chrono::nanoseconds( 8333333 ) );
      t = std::chrono::steady_clock::now();

      glfwPollEvents();

      // exp: FPS logging
      auto delta_nano = std::chrono::duration_cast< std::chrono::nanoseconds >(
        t - last_t ).count();
      last_t = t;

      now_fps = ( 1.e9 / delta_nano );
      if( avg_fps == 0 ) { avg_fps = now_fps; }
      avg_fps = ( ( avg_fps * ( avg_window - 1. ) ) + now_fps ) / avg_window;
//      LOG_DEBUG( "!!! FPS: " << now_fps << "(avg: " << avg_fps << ")" );
    }
    LOG_DEBUG( "Exited main loop" );
  }

  void
  cleanUp()
  {
    if( m_vk_logical_device )
    {
      // Logical device queues are implicitly cleaned up when their respective
      // logical device is
      // destroyed.
      LOG_INFO( "Destroying logical device" );
      vkDestroyDevice( m_vk_logical_device, nullptr );
    }
    if( m_vk_debug_messenger )  // null if not initialized.
    {
      LOG_DEBUG( "Destroying vulkan debug messenger" );
      vk_destroyDebugMessenger( this->m_vk_instance_handle,
                                this->m_vk_debug_messenger );
    }
    if( m_vk_surface )
    {
      LOG_DEBUG( "Destroying vulkan surface" );
      vkDestroySurfaceKHR( m_vk_instance_handle, m_vk_surface, nullptr );
    }
    if( m_vk_instance_handle )
    {
      LOG_DEBUG( "Destroying vulkan instance" );
      vkDestroyInstance( this->m_vk_instance_handle, nullptr );
      // physical device implicitly destroyed with instance.
      this->m_vk_physical_device = VK_NULL_HANDLE;
    }
    if( m_window )
    {
      LOG_DEBUG( "Destroying GLFW window instance" );
      glfwDestroyWindow( this->m_window );
      this->m_window = nullptr;
      glfwTerminate();
    }
  }
};

int
main()
{
  HelloTriangleApp app;
  // Lets not eat exceptions for now...
  app.run();
  LOG_DEBUG( "Exiting successfully!" );
  return EXIT_SUCCESS;
}
