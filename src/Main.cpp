#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include "volk.h"
#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include "Common.h"
#include "MonkeScene.h"

#include "spirv_reflect.h"


int main(int argc, char* argv[]) {
    MonkeScene scen;
    scen.Run();
    return EXIT_SUCCESS;
}
