#pragma once

#include <filesystem>
#include <unordered_map>

#include "10_Core/VuCommon.h"
#include "12_VuMakeCore/VuGraphicsPipeline.h"

#include "VuMaterial.h"

namespace Vu
{
    struct VuRenderPass;
    struct VuGraphicsShaderCreateInfo
    {
        Path         vertexShaderPath   = {};
        Path         fragmentShaderPath = {};
        VkRenderPass renderPass         = {};
    };

    struct VuShader
    {
        VuDevice*    vuDevice;
        Path         vertexShaderPath;
        Path         fragmentShaderPath;
        VuRenderPass* vuRenderPass;

        time_t         lastModifiedTime     = 0;
        VkShaderModule vertexShaderModule   = {};
        VkShaderModule fragmentShaderModule = {};

        std::unordered_map<MaterialSettings, VuGraphicsPipeline> compiledPipelines;

        void init(VuDevice* vuDevice, Path vertexShaderPath, Path fragmentShaderPath, VuRenderPass* vuRenderPass);

        void uninit();

        void tryRecompile();

        //get compiled pipeline handle if present, id not, compile and get
        VuGraphicsPipeline& requestPipeline(MaterialSettings materialSettings);

        static Path compileToSpirv(const Path& shaderCodePath);

        static VkShaderModule createShaderModule(VuDevice* vuDevice, const void* code, size_t size);
    };
}
