#pragma once
#include <vector>
#include "VuMaterial.h"

#include "10_Core/VuCommon.h"


namespace Vu
{
    struct VuGraphicsShaderCreateInfo
    {
        path         vertexShaderPath   = {};
        path         fragmentShaderPath = {};
        VkRenderPass renderPass         = {};
    };

    struct VuShader
    {
        VuDevice*    vuDevice;
        path         vertexShaderPath;
        path         fragmentShaderPath;
        VkRenderPass renderPass;

        time_t         lastModifiedTime     = 0;
        VkShaderModule vertexShaderModule   = {};
        VkShaderModule fragmentShaderModule = {};

        std::vector<VuMaterial> materials = {};

        static path compileToSpirv(const path& shaderCodePath);

        void init(VuDevice* vuDevice, path vertexShaderPath, path fragmentShaderPath, VkRenderPass renderPass);

        void uninit();

        void tryRecompile();

        //returns material Index
        uint32 createMaterial();


        static VkShaderModule createShaderModule(VuDevice* vuDevice, const void* code, size_t size);
    };
}
