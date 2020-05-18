#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include <vulkan/vulkan.h>


class HelloTriangleApp
{
public:
    ~HelloTriangleApp() = default;

    void run()
    {
        initVulkan();
        mainLoop();
        cleanUp();
    }

private:
    void initVulkan()
    {
    }

    void mainLoop()
    {
    }

    void cleanUp()
    {
    }
};


int main()
{
    HelloTriangleApp app;
    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
