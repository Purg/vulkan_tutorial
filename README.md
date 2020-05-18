Code produced from folling the Vulkan API tutorial here:

    https://vulkan-tutorial.com/

# CLion Notes
Toolchain: For parity with using the terminal, the global default toolchain
has been renamed "System" and uses the system installed CMake and GDB.

## Integration with Vulkan SDK
The locally unpacked Vulkan SDK from LunarG is configured for use via CMake
options specified under `Settings` -> `Build, Execution, Deployment` -> `CMake`
-> `Debug` -> `CMake Options`. Here, `Vulkan_LIBRARY` and `Vulkan_INCLUDE_DIR`
are explicitly specified with absolute paths.

Additionally, the `LD_LIBRARY_PATH` and `VK_LAYER_PATH` environment variables
are specified.
