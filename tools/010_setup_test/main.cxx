/**
 * A "make sure everything works" tool, described at the stort of the tutorial
 *
 * https://vulkan-tutorial.com/Development_environment#page_Setting-up-a-makefile-project
 */
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>

// This is technically implicitly included by glfw3 with the above DEFINE, but
// I hate implicitness.
#include <vulkan/vulkan.h>

#include <iostream>

#define LOG_INFO( msg ) std::cerr << "[INFO] " << msg << std::endl;

int
main()
{
  glfwInit();

  glfwWindowHint( GLFW_CLIENT_API, GLFW_NO_API );

  GLFWwindow* window = glfwCreateWindow( 800, 600, "Vulkan m_window", nullptr,
                                         nullptr );

  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

  LOG_INFO( extensionCount << " extensions supported" );

  glm::mat4 matrix;
  glm::vec4 vec;
  (void) ( matrix * vec );

  LOG_INFO( "Window should open and wait until closed by you (click the (X) "
            "button.)" );
  while( !glfwWindowShouldClose( window ) )
  {
    glfwPollEvents();
  }

  LOG_INFO( "The window should now be closed and the program should exit "
            "without error." );
  glfwDestroyWindow( window );

  glfwTerminate();

  return 0;
}
