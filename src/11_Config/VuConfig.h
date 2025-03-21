#pragma once
#include "10_Core/VuCommon.h"
#include "10_Core/VuPlatform.h"

//Matrix Layout : Column Major
//Coordinate System : Left handed, +Y up, +Z forward, +X right
//Unit Scale: 1 unit = 1 meter
//Angle Unit: Radian
//Euler angles order: YXZ (Yaw,Pitch,Roll)

//Material System: Shader = spirv, material = pipelines, material data = bindless indices + basic data



namespace Vu
{
    struct VuBindlessConfigInfo
    {
        uint32 uboBinding;
        uint32 samplerBinding;
        uint32 sampledImageBinding;
        uint32 storageImageBinding;
        uint32 storageBufferBinding;

        uint32 uboCount;
        uint32 samplerCount;
        uint32 sampledImageCount;
        uint32 storageImageCount;
        uint32 storageBufferCount;
    };
}

namespace Vu::config
{
    constexpr uint32 MAX_FRAMES_IN_FLIGHT = 2;

    // constexpr uint32 MAX_STORAGE_BUFFER_COUNT = 2;
    // constexpr uint32 MAX_SAMPLER_COUNT = 2;
    // constexpr uint32 MAX_SAMPLED_IMAGE_COUNT = 2;


    constexpr VuBindlessConfigInfo BINDLESS_CONFIG_INFO{
        .uboBinding = 0,
        .samplerBinding = 1,
        .sampledImageBinding = 2,
        .storageImageBinding = 3,
        .storageBufferBinding = 4,
        .uboCount = 1 ,
        .samplerCount = 4096 ,
        .sampledImageCount = 4096 ,
        .storageImageCount = 4096 ,
        .storageBufferCount = 4096 ,
    };

    constexpr uint32 SHADER_COUNT    = 256;
    constexpr uint32 MATERIAL_COUNT  = 1024;
    constexpr uint32 PUSH_CONST_SIZE = 256;

#ifdef NDEBUG
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = false;
#else
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = true;
#endif

    inline static Vector<const char*> VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    inline static std::vector<const char*> INSTANCE_EXTENSIONS = {
#ifndef  NDEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
    };

    inline static std::vector<const char*> DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
        VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
    };

    constexpr char SHADER_COMPILER_PATH[] = "bin" PLATFORM_SPECIFIC_PATH "/slang/slangc.exe";
    //constexpr char SHADER_COMPILER_PATH[] = "bin\\windows-x86_64\\slang\\slangc.exe";
}
