#pragma once
#include <filesystem>

#include "vulkan/vulkan.hpp"

#include "10_Core/VuPlatform.h"

//Matrix Layout : Column Major
//Coordinate System : Left handed, +Y up, +Z forward, +X right
//Unit Scale: 1 unit = 1 meter
//Angle Unit: Radian, Degree in UI but immediately converted to Radian on fetch
//Euler angles order: YXZ (Yaw,Pitch,Roll)

//Material System:
//Shader = spirv, and contains hashmap of compiled pipelines
//material data = bindless indices + basic data
//material = contain handle shader, and handle to material data


namespace Vu::config
{

constexpr u32 START_WIDTH  = 1280u;
constexpr u32 START_HEIGHT = 720u;

constexpr u32 MAX_FRAMES_IN_FLIGHT = 2u;

constexpr vk::DeviceSize MATERIAL_DATA_SIZE = 64u;
constexpr u32            PUSH_CONST_SIZE    = 256u;

constexpr u32 DEVICE_MAX_IMAGE_COUNT         = 256u;
constexpr u32 DEVICE_MAX_SAMPLER_COUNT       = 256u;
constexpr u32 DEVICE_MAX_BUFFER_COUNT        = 256u;
constexpr u32 DEVICE_MAX_SHADER_COUNT        = 256u;
constexpr u32 DEVICE_MAX_MATERIAL_DATA_COUNT = 256u;
constexpr u32 DEVICE_MAX_MATERIAL_COUNT      = 256u;

constexpr u32 BINDLESS_UNIFORM_BUFFER_COUNT = 1u;
constexpr u32 BINDLESS_SAMPLER_COUNT        = 256u;
constexpr u32 BINDLESS_SAMPLED_IMAGE_COUNT  = 256u;
constexpr u32 BINDLESS_STORAGE_IMAGE_COUNT  = 256u;
constexpr u32 BINDLESS_STORAGE_BUFFER_COUNT = 256u;

constexpr u32 BINDLESS_UNIFORM_BUFFER_BINDING = 0u;
constexpr u32 BINDLESS_SAMPLER_BINDING        = 1u;
constexpr u32 BINDLESS_SAMPLED_IMAGE_BINDING  = 2u;
constexpr u32 BINDLESS_STORAGE_IMAGE_BINDING  = 3u;
constexpr u32 BINDLESS_STORAGE_BUFFER_BINDING = 4u;

inline static std::vector<const char*> VALIDATION_LAYERS = {
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

#ifdef NDEBUG
    constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = false;
#else
constexpr bool ENABLE_VALIDATION_LAYERS_LAYERS = true;
#endif


inline std::filesystem::path getShaderCompilerPath()
{
    std::filesystem::path shaderCompilerPath = "bin";

    appendTargetVariablePath(shaderCompilerPath);
    shaderCompilerPath /= "slang/slangc";
    appendTargetVariableExtension(shaderCompilerPath);
    return shaderCompilerPath;
}
}
