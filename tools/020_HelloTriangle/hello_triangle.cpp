#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#include <myengine/logging.h>
#include <myengine/vulkan/instance.h>


#define VK_EXT_debug_utils_NAME "VK_EXT_debug_utils"


/**
 * Get the required vulkan extensions by name from GLFW's API.
 *
 * @return Vector of required extensions by string name.
 */
std::vector<char const *>
glfw_get_required_vulkan_extensions()
{
  uint32_t count = 0;
  char const **name_array;
  name_array = glfwGetRequiredInstanceExtensions( &count );
  return std::vector<char const *>( name_array, name_array + count );
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
      : win_height( window_height ),
        win_width( window_width ),
        window( nullptr ),
        vk_instance_handle( nullptr )
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


private:
  uint32_t win_height, win_width;
  GLFWwindow *window;
  // A pointer to the empty `struct VkInstance_T` type.
  VkInstance vk_instance_handle;
  // Validation layers we'll use (static because tutorial)
  // -- ONLY USED IN INSTANCE CREATION WHEN IN DEBUG.
  std::vector<char const *> static_validation_layers = {
      "VK_LAYER_KHRONOS_validation",
  };

  /**
   * Initialize GLFW window instance to use.
   */
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
    this->window = glfwCreateWindow(
        win_width, win_height, HelloTriangleApp::APP_NAME,
        nullptr, nullptr
    );
  }

  void initVulkan()
  {
    createVulkanInstance();
  }

  void mainLoop()
  {
    LOG_DEBUG( "Starting main loop..." );
    while( !glfwWindowShouldClose( window ) )
    {
      glfwPollEvents();
    }
  }

  void cleanUp()
  {
    LOG_DEBUG( "Destroying vulkan instance" );
    vkDestroyInstance( this->vk_instance_handle, nullptr );
    LOG_DEBUG( "Destroying GLFW window instance" );
    glfwDestroyWindow( this->window );
    this->window = nullptr;
    glfwTerminate();
  }

  /**
   * Instantiate the Vulkan Instance
   *
   * This could probably be made into a utility func in the engine lib.
   * Could take parameters for the app_info stuff, as well as extensions and
   * validation layers.
   *
   * @throws std::runtime_error
   *   Requested instance extension or validation layer not currently supported.
   */
  void createVulkanInstance()
  {
    // out-of-order designated initializer apparently unimplemented in g++, so
    // not doing that here... sad...
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = HelloTriangleApp::APP_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
    app_info.pEngineName = "myengine";
    app_info.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
    // Maybe derive from external/CMake setting?
    app_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    std::vector<char const *> inst_extensions;
    std::vector<char const *> inst_validation_layers;

    // Request extensions //////////////////////////////////////////////////////
    // Enable the global extensions requested by the windowing system we're
    // using (GLFW).
    // NOTE: This part would get swapped out if moving to SDL.
    for( auto const &ext_name : glfw_get_required_vulkan_extensions() )
    {
      inst_extensions.push_back( ext_name );
    }
#ifndef NDEBUG
    inst_extensions.push_back( VK_EXT_debug_utils_NAME );
#endif

    // Request validation layers ///////////////////////////////////////////////
#ifndef NDEBUG
    inst_validation_layers.insert(
        inst_validation_layers.end(),
        static_validation_layers.cbegin(), static_validation_layers.cend() );
#endif

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
    VkResult result = vkCreateInstance( &create_info, nullptr,
                                        &vk_instance_handle );
    if( result != VK_SUCCESS )
    {
      std::stringstream ss;
      ss << "Failed to initialize Vulkan instance with error " << result;
      throw std::runtime_error( ss.str() );
    }
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
