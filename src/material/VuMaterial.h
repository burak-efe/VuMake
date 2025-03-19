#pragma once

#include "Common.h"

#include "VuGraphicsPipeline.h"
#include "VuMaterialDataPool.h"
#include "VuPools.h"
#include "VuTypes.h"


namespace Vu
{
    struct VuMaterialDataPool;

    struct VuMaterialCreateInfo
    {
        VkShaderModule      vertexShaderModule;
        VkShaderModule      fragmentShaderModule;
        VkRenderPass        renderPass;
        VuMaterialDataPool* materialDataPool;
    };

    //Material owns the pipeline, uses shared material data
    //when parent shader recompiled, it should be recompiled too
    struct VuMaterial
    {
        VuMaterialCreateInfo         lastCreateInfo;
        VuGraphicsPipeline           vuPipeline;
        VuHandle2<uint32> materialData;

        void init(const VuMaterialCreateInfo& createInfo);

        void recompile(const VuMaterialCreateInfo& createInfo);

        void uninit();

        void bindPipeline(const VkCommandBuffer& commandBuffer) const;


        GPU_PBR_MaterialData* getMaterialData();
    };
}
