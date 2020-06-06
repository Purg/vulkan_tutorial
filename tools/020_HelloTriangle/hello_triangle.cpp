#include <cstdlib>
#include <exception>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <myengine/logging.hpp>
#include <myengine/glfw.hpp>
#include <myengine/vulkan.hpp>


#define VK_EXT_debug_utils_NAME "VK_EXT_debug_utils"


struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
};


/**
 * Access the global static list of validation layers to be used.
 *
 * This is a singleton/construct-on-first-use idiom.
 *
 * @return The vector of string validation layer names.
 */
std::vector<char const *> const &
STATIC_INSTANCE_VALIDATION_LAYERS()
{
  static std::vector<char const *> const v = {
      "VK_LAYER_KHRONOS_validation",
  };
  return v;
}


/**
 * Callback linkup with debug logging.
 *
 * The use of this callback at all is intended to be conditional on !NDEBUG.
 * With that, I guess "verbose" messages are on the table to be output?
 *
 * NOTE: The *real* way to go here is probably for this while thing to take in
 * parametrization for multiple levels of verbosity for finer grain control
 * (e.g. want info+ logging, but just for validation types, etc.)
 */
VKAPI_ATTR VkBool32 VKAPI_CALL
vk_debug_messenger_logging_hook(
    VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
    VkDebugUtilsMessageTypeFlagsEXT msg_type,
    VkDebugUtilsMessengerCallbackDataEXT const *p_callback_data,
    void *p_user_data )
{
  std::string s_severity =
      vk::to_string( (vk::DebugUtilsMessageSeverityFlagBitsEXT)msg_severity ),
      s_type =
      vk::to_string( (vk::DebugUtilsMessageTypeFlagBitsEXT)msg_type );
  // TODO: Lambda this function to parameterize the logging prefix used here?
  //       or nest function with template pattern for such customization via
  //       thin wrappers. Basically want to differentiate messages between
  //       different messengers. Currently I see that I will need to register
  //       different callback functions.
  std::cerr << "Khronos"
            << "[" << s_severity << "]"
            << "[type::" << s_type << "] "
            << p_callback_data->pMessage << std::endl;
  return VK_FALSE;
}


/**
 * Fill in a given create-info struct for a debug messenger given this
 * tutorial/app context.
 *
 * @param [in,out] create_info
 *   Info struct to update values of.
 */
void
vk_debug_messenger_create_info_fill( VkDebugUtilsMessengerCreateInfoEXT &create_info )
{
  create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT  // maybe remove this later if its excessive?
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
      | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
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
 * @sa `createVulkanInstance` for note on `[[ nodiscard ]]` attribute.
 */
[[nodiscard]] VkDebugUtilsMessengerEXT
vk_createDebugMessenger( VkInstance const &instance )
{
  // As long as I go in order and don't skip anything, gcc is OK...
  VkDebugUtilsMessengerCreateInfoEXT create_info = {};
  vk_debug_messenger_create_info_fill( create_info );
  auto create_func = (PFN_vkCreateDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
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
    ss << "Failed to create debug messenger! "
       << vk::to_string( static_cast<vk::Result>(create_result) );
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
vk_destroyDebugMessenger( VkInstance const &instance,
                          VkDebugUtilsMessengerEXT const &messenger )
{
  auto destroy_func = (PFN_vkDestroyDebugUtilsMessengerEXT)
      vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );
  if( messenger != nullptr && destroy_func != nullptr )
  {
    destroy_func( instance, messenger, nullptr );
  }
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
  char const *APP_NAME = "HelloTriangle";

  HelloTriangleApp()
      : HelloTriangleApp( 600, 800 )
  {}

  explicit HelloTriangleApp( uint32_t window_height, uint32_t window_width )
      : m_win_height( window_height ),
        m_win_width( window_width ),
        m_window( nullptr ),
        m_vk_instance_handle( VK_NULL_HANDLE ),
        m_vk_debug_messenger( VK_NULL_HANDLE ),
        m_vk_physical_device( VK_NULL_HANDLE ),
        m_vk_logical_device( VK_NULL_HANDLE ),
        m_vk_queue_graphics( VK_NULL_HANDLE )
  {}

  ~HelloTriangleApp() = default;

  void run()
  {
    initWindow();
    initVulkan();
    mainLoop();
    // Call in destructor instead?
    cleanUp();
  }


private:  // variables
  uint32_t m_win_height, m_win_width;
  GLFWwindow *m_window;
  // `VkInstance` *is* a pointer: to the empty `struct VkInstance_T` type.
  VkInstance m_vk_instance_handle;
  // Optional pointer to a debug messenger. May be null.
  VkDebugUtilsMessengerEXT m_vk_debug_messenger;
  // Opaque handle to the physical device to use.
  // Implicitly destroyed with the vulkan instance.
  VkPhysicalDevice m_vk_physical_device;
  // Opaque handle to the logical device for this app.
  VkDevice m_vk_logical_device;
  // Opaque handles for queues
  VkQueue m_vk_queue_graphics;

private:  // methods
  /// Initialize GLFW window instance to use.
  void initWindow()
  {
    int glfw_init_ret = glfwInit();
    if( glfw_init_ret != GLFW_TRUE )
    {
      std::stringstream ss;
      ss << "glfwInit() returned failure code " << glfw_init_ret;
      throw std::runtime_error( ss.str() );
    }
    // Prevent creation of OpenGL context.
    glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );
    // Disable window resizing.
    glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

    // To make full-screen, `monitor` will need to be specified and the
    // specified size will be the resolution, otherwise will be in windowed
    // mode.
    this->m_window = glfwCreateWindow(
        m_win_width, m_win_height, HelloTriangleApp::APP_NAME,
        nullptr, nullptr
    );
  }

  void initVulkan()
  {
    m_vk_instance_handle = createVulkanInstance();
#ifndef NDEBUG
    m_vk_debug_messenger = vk_createDebugMessenger( this->m_vk_instance_handle );
#endif
    m_vk_physical_device = pickPhysicalDevice( m_vk_instance_handle );

    // technically a duplicate call, see `is_suitable_device`
    QueueFamilyIndices qf_indices = {};
    find_queue_families( m_vk_physical_device, qf_indices );

    m_vk_logical_device = create_logical_device( m_vk_physical_device, qf_indices );

    LOG_DEBUG( "Let's grab the logical device's graphics queue." );
    // Hardcoded `0` here "because we're only creating a single queue from this family."
    // I think this is tied to the `VkDeviceQueueCreateInfo.queueCount` value which is currently
    // set to `1` in `create_logical_device`.
    vkGetDeviceQueue( m_vk_logical_device, qf_indices.graphicsFamily.value(), 0,
                      &m_vk_queue_graphics );
  }

  void mainLoop()
  {
    LOG_DEBUG( "Starting main loop..." );
    while( !glfwWindowShouldClose( m_window ) )
    {
      glfwPollEvents();
    }
  }

  void cleanUp()
  {
    if( m_vk_logical_device )
    {
      // Logical device queues are implicitly cleaned up when their respective logical device is
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

  //////////////////////////////////////////////////////////////////////////////////////////////////

  /// App-specific validation layer addition logic
  void
  populate_app_validation_layers( std::vector<char const *> &validation_layers ) const
  {
    // We only want to add validation layers if we're flagged for debug.
#ifndef NDEBUG
    auto p_validation_layers = &STATIC_INSTANCE_VALIDATION_LAYERS();
    validation_layers.insert( validation_layers.end(),
                              p_validation_layers->cbegin(),
                              p_validation_layers->cend() );
#endif
  }

  /// Instantiate the Vulkan Instance
  /**
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
   * Of course this method could probably just be static...
   *
   * @throws std::runtime_error
   *   Requested instance extension or validation layer not currently supported.
   */
  [[nodiscard]] VkInstance
  createVulkanInstance() const
  {
    // out-of-order designated initializer apparently unimplemented in g++, so
    // not doing that here... sad...
    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = HelloTriangleApp::APP_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
    app_info.pEngineName = "myengine";
    app_info.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
    // Maybe derive from external/CMake setting?
    app_info.apiVersion = VK_API_VERSION_1_1;

    // TODO: Could probably function up the create_info and
    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    std::vector<char const *> inst_extensions;
    std::vector<char const *> inst_validation_layers;

    // Register "next" pointer to debug messenger if in debug mode /////////////
    // This is to allow debug messenger to activate for instance
    // creation/destruction since the "main" debug messenger is created *after*
    // instance creation.
#ifndef NDEBUG
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
    for( auto const &ext_name : myengine::glfw::glfw_get_required_vulkan_extensions() )
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
    for( auto const &ext_name : inst_extensions )
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
    for( auto const &layer_name : inst_validation_layers )
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
      ss << "Failed to initialize Vulkan instance with error code " << result
         << " (" << vk::to_string( static_cast<vk::Result>(result) ) << ")";
      throw std::runtime_error( ss.str() );
    }
    return instance;
  }

  /// App-specific physical device selection criterion.
  /**
   * @return Input device is suitable.
   */
  [[nodiscard]]
  static bool
  is_suitable_device( VkPhysicalDevice const &device )
  {
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures feats;
    vkGetPhysicalDeviceProperties( device, &props );
    vkGetPhysicalDeviceFeatures( device, &feats );
    LOG_DEBUG( "Considering device (" << props.deviceID << ") '" << props.deviceName << "'" );
    // See https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families#page_Base-device-suitability-checks
    // for alternative examples.

    QueueFamilyIndices indices = {};
    find_queue_families( device, indices );
    return indices.graphicsFamily.has_value();
  }

  /// Decide which physical device is to be used.
  /**
   * @throws std::runtime_error No physical devices that pass the filter criterion.
   * @return A singular physical device handle to be used.
   */
  [[nodiscard]] VkPhysicalDevice
  pickPhysicalDevice( VkInstance const &instance ) const
  {
    std::vector<VkPhysicalDevice> device_vec = myengine::vulkan::get_physical_devices(
        m_vk_instance_handle,
        []( VkPhysicalDevice const &device ) -> bool
        {
          return is_suitable_device( device );
        }
    );
    if( device_vec.empty() )
    {
      throw std::runtime_error( "Zero physical devices discovered post filter." );
    }
    // This is a dumb initial pass. Something more should be done instead of just selecting th
    // first one in the list.
    return device_vec[0];
  }

  /// Update the given `QueueFamilyIndices` structure reference with the device's supported queue
  /// families.
  /**
   * This currently assumes that there is only one queue per tracked index type, or that the last
   * queue family found per tracked index type is acceptable.
   *
   * @param [in] device Physical device to query the queue properties of.
   * @param [out] indices Structure to set queue indices to.
   */
  static void
  find_queue_families( VkPhysicalDevice const &device,
                       QueueFamilyIndices &indices )
  {
    auto queue_props_vec = myengine::vulkan::get_device_queue_family_properties( device );
    int i = 0;
    for( auto const &queue_props : queue_props_vec )
    {
      if( queue_props.queueFlags & VK_QUEUE_GRAPHICS_BIT )
      {
        // Why is this getting set to the last index supporting graphics? Why not the first?
        // In general this method needs more fleshing out for more criterion.
        indices.graphicsFamily = i;
      }
      ++i;
    }
  }

  /// Create logical devices for this app.
  /**
   * @param [in] physical_device Physical physical_device from which the logical physical_device
   *   should be created.
   * @param [in] qf_indices Queried queue family indices for the given device as from
   *   `find_queue_families`.
   *
   * @throws std::bad_optional_access If we request the index of a queue family that the physical
   *    device does not support.
   * @throws std::runtime_error Failed to create the logical device.
   *
   * @returns Opaque handle to the newly created logical device.
   */
  VkDevice
  create_logical_device( VkPhysicalDevice const &physical_device,
                         QueueFamilyIndices const &qf_indices )
  {
    VkDeviceQueueCreateInfo q_create_info = {};
    q_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    q_create_info.queueFamilyIndex = qf_indices.graphicsFamily.value();
    // Generally only need a few concurrent queues as most.
    // Command buffers submitted to a single queue are executed ("started") in order relative to
    // each other. Commands submitted to different queues are unordered relative to each other
    // without explicit synchronization (see `VkSemaphore`). Can only submit to a queue from one
    // thread at a time (or across multiple with "external" synchronization), while different
    // threads may submit to different queues simultaneously.
    //
    // So far I think the intent here is that it doesn't make sense to submit to a graphics queue
    // asynchronously (without more complicated logic/synchronization at least).
    q_create_info.queueCount = 1;
    // A [0, 1] (inclusive) priority value for the queue to influence the scheduling of command
    // buffer execution. I'm interpreting that 1.0 is the maximum priority.
    float q_priority = 1.f;
    q_create_info.pQueuePriorities = &q_priority;

    // From Tutorial: Right now we don't need anything special, so we can simply define it and
    // leave everything to VK_FALSE. We'll come back to this structure once we're about to start
    // doing more interesting things with Vulkan.
    VkPhysicalDeviceFeatures device_features = {};

    // Creation info struct for the logical device.
    VkDeviceCreateInfo d_create_info = {};
    d_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    d_create_info.pQueueCreateInfos = &q_create_info;
    d_create_info.queueCreateInfoCount = 1;
    // Device specific layers are currently deprecated as of at least vulkan 1.1. The tutorial
    // recommends setting them anyway to support older vulkan implementations. OK FINE...
    std::vector<char const *> device_validation_layers;
    populate_app_validation_layers( device_validation_layers );
    d_create_info.enabledLayerCount = device_validation_layers.size();
    d_create_info.ppEnabledLayerNames = device_validation_layers.data();
    // Probably going to revisit this in a later tutorial chapter. Mentioned "VK_KHR_swapchain".
    d_create_info.enabledExtensionCount = 0;
    // The features
    d_create_info.pEnabledFeatures = &device_features;

    VkDevice logical_device = VK_NULL_HANDLE;
    VkResult res = vkCreateDevice( physical_device, &d_create_info, nullptr, &logical_device );
    if( res != VK_SUCCESS )
    {
      std::stringstream ss;
      ss << "Failed to create logical device: " << vk::to_string( (vk::Result)res );
      throw std::runtime_error( ss.str() );
    }
    return logical_device;
  }
};


int
main()
{
  HelloTriangleApp app;
  try
  {
    app.run();
  }
  catch( const std::exception &e )
  {
    std::cerr << "Caught exception (type: " << typeid( e ).name() << "): "
              << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  LOG_DEBUG( "Exiting successfully!" );
  return EXIT_SUCCESS;
}
