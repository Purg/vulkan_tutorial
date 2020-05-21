#include <cstdlib>
#include <exception>
#include <iostream>
#include <sstream>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

/*******************************************************************************
 * Some simple logging stuff for now.
 */
#define LOG_INFO( msg ) \
  do \
  { \
    std::cerr << "[BASIC][ INFO] " << msg << std::endl; \
  } while(false)

#ifdef NDEBUG
#define LOG_DEBUG( msg )
#else
#define LOG_DEBUG( msg ) \
  do \
  { \
    std::cerr << "[BASIC][DEBUG] " << msg << std::endl; \
  } while(false)
#endif

/*******************************************************************************
 * Our home for the tutorial: a hello-world like class.
 */
class HelloTriangleApp
{
public:
  char const *APP_NAME = "HelloTriangle";

  HelloTriangleApp()
      : HelloTriangleApp(600, 800)
  {}

  explicit HelloTriangleApp( uint32_t window_height, uint32_t window_width )
      : win_height(window_height),
        win_width(window_width),
        window(nullptr),
        vk_instance_handle(nullptr)
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
      throw std::runtime_error(ss.str());
    }
    // Prevent creation of OpenGL context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Disable window resizing.
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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
    while( !glfwWindowShouldClose(window) )
    {
      glfwPollEvents();
    }
  }

  void cleanUp()
  {
    LOG_DEBUG( "Destroying vulkan instance" );
    vkDestroyInstance(this->vk_instance_handle, nullptr);
    LOG_DEBUG( "Destroying GLFW window instance" );
    glfwDestroyWindow(this->window);
    this->window = nullptr;
    glfwTerminate();
  }

  void createVulkanInstance()
  {
    // out-of-order designated initializer apparently unimplemented in g++, so
    // not doing that here... sad...
    VkApplicationInfo app_info;
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = HelloTriangleApp::APP_NAME;
    app_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.pEngineName = "No engine? WhAt Is ThIs?";
    app_info.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app_info.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    // Leave disabled the validation layers for the moment.
    create_info.enabledLayerCount = 0;

    // Enable the global extensions requested by the windowing system we're
    // using (GLFW).
    // NOTE: This part would get swapped out if moving to SDL.
    uint32_t glfwExtensionCount = 0;
    char const **glfwExtensionNameArray;
    glfwExtensionNameArray =
        glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    LOG_DEBUG("GLFW Requested extension names:");
    for( uint32_t i = 0; i < glfwExtensionCount; ++i )
    {
      LOG_DEBUG(".. " << glfwExtensionNameArray[i]);
    }

    // Actually create the instance...
    VkResult result = vkCreateInstance(&create_info, nullptr,
                                       &vk_instance_handle);
    if( result != VK_SUCCESS )
    {
      std::stringstream ss;
      ss << "Failed to initialize Vulkan instance with error " << result;
      throw std::runtime_error(ss.str());
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
    std::cerr << "Caught exception (type: " << typeid(e).name() << "): "
              << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  LOG_DEBUG( "Exiting successfully!" );
  return EXIT_SUCCESS;
}
