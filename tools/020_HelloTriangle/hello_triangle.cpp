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
  std::vector<char const *> validation_layers = {
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
   */
  void createVulkanInstance()
  {
    // out-of-order designated initializer apparently unimplemented in g++, so
    // not doing that here... sad...
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = HelloTriangleApp::APP_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
    app_info.pEngineName = "No myengine? WhAt Is ThIs?";
    app_info.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
    app_info.apiVersion = VK_API_VERSION_1_1;  // Maybe derive from external

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    // Enable the global extensions requested by the windowing system we're
    // using (GLFW).
    // NOTE: This part would get swapped out if moving to SDL.
    uint32_t glfwExtensionCount = 0;
    char const **glfwExtensionNameArray;
    glfwExtensionNameArray =
        glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
    LOG_DEBUG( "GLFW Requested extension names:" );
    for( uint32_t i = 0; i < glfwExtensionCount; ++i )
    {
      LOG_DEBUG( ".. " << glfwExtensionNameArray[i] );
    }
    create_info.enabledExtensionCount = glfwExtensionCount;
    create_info.ppEnabledExtensionNames = glfwExtensionNameArray;

    // Leave disabled the validation layers for the moment.
#ifdef NDEBUG
    create_info.enabledLayerCount = 0;
#else
    if( !myengine::vulkan::check_instance_layer_support( this->validation_layers ) )
    {
      throw std::runtime_error( "Validation layers requested but not all were "
                                "enumerated as available" );
    }
    create_info.enabledLayerCount = (uint32_t) (validation_layers.size());
    create_info.ppEnabledLayerNames = validation_layers.data();
#endif

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
