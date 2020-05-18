#include <cstdlib>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>


class HelloTriangleApp
{
public:
  char const* WIN_TITLE = "HelloTriangle";

  explicit HelloTriangleApp(uint32_t window_height = 600,
                            uint32_t window_width = 800)
      : win_height(window_height), win_width(window_width), window(nullptr)
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

  void initWindow()
  {
    int glfw_init_ret = glfwInit();
    if (glfw_init_ret != GLFW_TRUE)
    {
      std::ostringstream ss;
      ss << "glfwInit() returned failure code " << glfw_init_ret;
      throw std::runtime_error(ss.str());
    }
    // Prevent creation of OpenGL context.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    // Disable window resizing
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    // To make full-screen, `monitor` will need to be specified and the
    // specified size will be the resolution, otherwise will be in windowed
    // mode.
    this->window = glfwCreateWindow(
        win_width, win_height, HelloTriangleApp::WIN_TITLE,
        nullptr, nullptr
    );
  }

  void initVulkan()
  {}

  void mainLoop()
  {
    while (!glfwWindowShouldClose(window))
    {
      glfwPollEvents();
    }
  }

  void cleanUp()
  {
    glfwDestroyWindow(this->window);
    this->window = nullptr;
    glfwTerminate();
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
  catch (const std::exception &e)
  {
    std::cerr << "Caught exception (type: " << typeid(e).name() << "): "
              << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
