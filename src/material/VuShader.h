#pragma once
#include <vector>
#include "Common.h"
#include "VuMaterial.h"



namespace Vu
{
    struct VuGraphicsShaderCreateInfo
    {
        Path vertexShaderPath   = {};
        Path fragmentShaderPath = {};

        VkRenderPass renderPass = {};
    };

    struct VuShader
    {
        VuGraphicsShaderCreateInfo lastCreateInfo       = {};
        time_t                     lastModifiedTime     = 0;
        VkShaderModule             vertexShaderModule   = {};
        VkShaderModule             fragmentShaderModule = {};

        std::vector<VuMaterial> materials = {};

        static Path compileToSpirv(const Path& shaderCodePath);


        void init(const VuGraphicsShaderCreateInfo& createInfo);

        void uninit();

        void tryRecompile();

        //returns material Index
        uint32 createMaterial();


        static VkShaderModule createShaderModule(const void* code, size_t size);
    };
}
