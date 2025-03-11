#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include "volk.h"
#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
// #define GLM_FORCE_RADIANS
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_ENABLE_EXPERIMENTAL

#include "Common.h"

#define BUDDY_ALLOC_IMPLEMENTATION
#include "buddy_alloc.h"

#include "Scene0.h"



int main(int argc, char* argv[]) {
    Vu::Scene0 scen{};

    try {
        scen.Run();
    } catch (const std::exception& e) {
        std::puts(e.what());
        system("pause");
    }
    return EXIT_SUCCESS;
}
