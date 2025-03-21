#pragma once

#include <vector>
#include "10_Core/VuCommon.h"
#include "VuMaterial.h"

namespace Vu
{
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
        VkRenderPass renderPass;

        time_t         lastModifiedTime     = 0;
        VkShaderModule vertexShaderModule   = {};
        VkShaderModule fragmentShaderModule = {};

        std::vector<VuMaterial> materials = {};



        static Path compileToSpirv(const Path& shaderCodePath);

        void init(VuDevice* vuDevice, Path vertexShaderPath, Path fragmentShaderPath, VkRenderPass renderPass);

        void uninit();

        void tryRecompile();

        //returns material Index
        uint32 createMaterial();


        static VkShaderModule createShaderModule(VuDevice* vuDevice, const void* code, size_t size);
    };
}
