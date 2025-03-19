#pragma once
#include <vector>
#include "Common.h"
#include "VuMaterial.h"



namespace Vu
{
    struct VuGraphicsShaderCreateInfo
    {
        path vertexShaderPath   = {};
        path fragmentShaderPath = {};

        VkRenderPass renderPass = {};
    };

    struct VuShader
    {
        VuGraphicsShaderCreateInfo lastCreateInfo       = {};
        time_t                     lastModifiedTime     = 0;
        VkShaderModule             vertexShaderModule   = {};
        VkShaderModule             fragmentShaderModule = {};

        std::vector<VuMaterial> materials = {};

        static path compileToSpirv(const path& shaderCodePath);


        void init(const VuGraphicsShaderCreateInfo& createInfo);

        void uninit();

        void tryRecompile();

        //returns material Index
        uint32 createMaterial(VuMaterialDataPool* materialDataPool);


        static VkShaderModule createShaderModule(const void* code, size_t size);
    };
}
