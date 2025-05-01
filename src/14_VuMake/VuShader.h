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
        path         vertexShaderPath;
        path         fragmentShaderPath;
        VkRenderPass renderPass;
    };

    struct VuShader
    {
        VuDevice*     vuDevice           = nullptr;
        path          vertexShaderPath   = "error";
        path          fragmentShaderPath = "error";
        VuRenderPass* vuRenderPass       = nullptr;

        time_t         lastModifiedTime     = 0;
        VkShaderModule vertexShaderModule   = {};
        VkShaderModule fragmentShaderModule = {};

        std::unordered_map<MaterialSettings, VuGraphicsPipeline> compiledPipelines = {};

        void init(VuDevice* vuDevice, path vertexShaderPath, path fragmentShaderPath, VuRenderPass* vuRenderPass);

        void uninit();

        void tryRecompile();

        //get compiled pipeline handle if present, id not, compile and get
        VuGraphicsPipeline& requestPipeline(MaterialSettings materialSettings);

        static path compileToSpirv(const path& shaderCodePath);

        static VkShaderModule createShaderModule(VuDevice* vuDevice, const void* code, size_t size);
    };
}
