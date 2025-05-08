#pragma once

#include <ctime>                               // for size_t, time_t
#include <unordered_map>                       // for unordered_map

#include <vulkan/vulkan_core.h>                // for vk::ShaderModule, vk::Rend...

#include "10_Core/Common.h"                  // for path
#include "12_VuMakeCore/VuGraphicsPipeline.h"  // for VuGraphicsPipeline
#include "VuMaterial.h"                        // for hash, MaterialSettings


namespace Vu
{
struct VuRenderPass;
struct VuDevice;

struct VuGraphicsShaderCreateInfo
{
    path         vertexShaderPath;
    path         fragmentShaderPath;
    vk::RenderPass renderPass;
};

struct VuShader
{
    VuDevice*     vuDevice           = nullptr;
    path          vertexShaderPath   = "error";
    path          fragmentShaderPath = "error";
    VuRenderPass* vuRenderPass       = nullptr;

    time_t         lastModifiedTime     = 0;
    vk::ShaderModule vertexShaderModule   = {};
    vk::ShaderModule fragmentShaderModule = {};

    std::unordered_map<MaterialSettings, VuGraphicsPipeline> compiledPipelines = {};

    void init(VuDevice* vuDevice, path vertexShaderPath, path fragmentShaderPath, VuRenderPass* vuRenderPass);

    void uninit();

    void tryRecompile();

    //get compiled pipeline handle if present, id not, compile and get
    VuGraphicsPipeline& requestPipeline(MaterialSettings materialSettings);

    static path compileToSpirv(const path& shaderCodePath);

    static vk::ShaderModule createShaderModule(VuDevice* vuDevice, const void* code, size_t size);
};
}
