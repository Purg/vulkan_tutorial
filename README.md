# Following a Tutorial
Code produced from following the Vulkan API tutorial here:

    https://vulkan-tutorial.com/
    
The code portion of this tutorial seems to be licensed CC0 1.0 Universal,
if I'm reading the source repository's README section about licenses
([here](https://github.com/Overv/VulkanTutorial#license)).
Please correct me about this if I'm interpreting this incorrectly.

# Dependencies
TODO: Write CMake to optionally automatically fetch/build required 3rd party libraries (see Fletch).

## Vulkan SDK
Using the LunarG Vulkan SDK found [here](https://vulkan.lunarg.com/sdk/home).
Seemingly the only reason I'm using this is for the bundled VkLayers.
Specifically using the latest version: `1.2.135.0`.
Mostly because I don't know better or otherwise, and this seems to work
currently with the Quadro P2000 on my laptop
([Nvidia says 1.2 is beta here](https://developer.nvidia.com/vulkan-driver)).
We'll see how this changes as I learn and try this on other GPUs.
I'm open to advice.

With the use of CMake, `find_package(Vulkan)` will possibly find a system
install of the vulkan development shared library and headers.
If that's desired, OK.
Otherwise adjust `Vulkan_INCLUDE_DIR` and `Vulkan_LIBRARY` to point to your
desired SDK parts.

## GLFW (3)
Currently following the tutorial and using GLFW for providing windowing.

**CentOS 8**
- Enable the PowerTools repository:
  `/etc/yum.repos.d/CentOS-PowerTools.repo`
  OR `sudo dnf config-manager --set-enabled PowerTools`
- `sudo dnf install glfw-devel`

## glm
OpenGL math header-only library.
Vulkan does not provide any on-build mathematics libraries or functions so we're using this to
provide that functionality.
I hear eigen is a possible alternative.

**CentOS 8**
- Enable the PowerTools repository:
  `/etc/yum.repos.d/CentOS-PowerTools.repo`
  OR `sudo dnf config-manager --set-enabled PowerTools`
- `sudo dnf install glm-devel`

# Building
Create an out of source directory (or `build` sub-directory) for build products.
Then to the usual CMake thing:
```bash
cd build
cmake ../
make -j$(nproc)
```

You may have to point `NOT-FOUND` required packages to their appropriate locations if
you have custom locations or using local LunarG Vulkan SDK binaries.


# CLion Notes
**Toolchain**:
For parity with using the terminal, I renamed the global default
toolchain "System" and uses the system installed CMake and GDB.
This facilitates being able to use \[c\]cmake on the clip without getting cmake
version errors.

## Integration with Vulkan SDK
The locally unpacked Vulkan SDK from LunarG is configured for use via CMake
options specified under `Settings` -> `Build, Execution, Deployment` -> `CMake`
-> `Debug` -> `CMake Options`. Here, `Vulkan_LIBRARY` and `Vulkan_INCLUDE_DIR`
are explicitly specified with absolute paths.

Additionally, the `LD_LIBRARY_PATH` and `VK_LAYER_PATH` environment variables
are specified.
