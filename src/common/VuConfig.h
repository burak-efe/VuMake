#pragma once
#include "Common.h"

namespace Vu::config {
    constexpr uint32 MAX_FRAMES_IN_FLIGHT = 2;

    constexpr uint32 UBO_BINDING = 0;
    constexpr uint32 STORAGE_BINDING = 1;
    constexpr uint32 SAMPLER_BINDING = 2;
    constexpr uint32 IMAGE_BINDING = 3;

    constexpr uint32 UNIFORM_COUNT = 1;
    constexpr uint32 STORAGE_COUNT = 65536;
    constexpr uint32 SAMPLER_COUNT = 65536;
    constexpr uint32 IMAGE_COUNT = 65536;
    constexpr uint32 SHADER_COUNT = 256;
    constexpr uint32 MATERIAL_COUNT = 1024;

#ifdef NDEBUG
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = false;
#else
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = true;
#endif

    const std::vector<const char *> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    constexpr char SHADER_COMPILER_PATH[] = "bin\\slang\\slangc.exe";

    constexpr std::array DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
    };

}
