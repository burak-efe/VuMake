#pragma once
#include "Common.h"

//Matrix Layout : Column Major
//Coordinate System : Left handed, +Y up, +Z forward, +X right
//Unit Scale: 1 unit = 1 meter
//Angle Unit: Radian
//Euler angles order: YXZ (Yaw,Pitch,Roll)

namespace Vu {
    struct VuBindlessConfigInfo {
        uint32 uboBinding = 0;
        uint32 samplerBinding = 1;
        uint32 sampledImageBinding = 2;
        uint32 storageImageBinding = 3;
        uint32 uniformBufferBinding = 4;
        uint32 storageBufferBinding = 5;

        uint32 uboCount = 1;
        uint32 samplerCount = 4096;
        uint32 sampledImageCount = 4096;
        uint32 storageImageCount = 4096;
        uint32 uniformBufferCount = 8;
        uint32 storageBufferCount = 4096;
    };

}

namespace Vu::config {

    constexpr uint32 MAX_FRAMES_IN_FLIGHT = 2;
    constexpr VuBindlessConfigInfo BINDLESS_CONFIG_INFO{
        .uboBinding = 0,
        .samplerBinding = 1,
        .sampledImageBinding = 2,
        .storageImageBinding = 3,
        .uniformBufferBinding = 4,
        .storageBufferBinding = 5,
        .uboCount = 1,
        .samplerCount = 4096,
        .sampledImageCount = 4096,
        .storageImageCount = 4096,
        .uniformBufferCount = 8,
        .storageBufferCount = 4096,
    };

    constexpr uint32 SHADER_COUNT = 256;
    constexpr uint32 MATERIAL_COUNT = 1024;

    constexpr uint32 PUSH_CONST_SIZE = 256;

#ifdef NDEBUG
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = false;
#else
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = true;
#endif

    inline static std::vector<const char *> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    inline static std::vector<const char *> INSTANCE_EXTENSIONS = {
#ifndef  NDEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        //VK_EXT_LAYER_SETTINGS_EXTENSION_NAME,
#endif
    };

    inline static std::vector<const char *> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        //VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
        //VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
        //VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME
    };

    constexpr char SHADER_COMPILER_PATH[] = "bin\\slang\\slangc.exe";


}
