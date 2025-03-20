#pragma once

#include "10_Core/VuCommon.h"
#include "VuGraphicsPipeline.h"
#include "VuPools.h"
#include "VuTypes.h"


namespace Vu
{
    struct VuDevice;

    //Material owns the pipeline, uses shared material data
    //when parent shader recompiled, it should be recompiled too
    struct VuMaterial
    {
        VuDevice*      vuDevice;
        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
        VkRenderPass   renderPass;

        VuGraphicsPipeline vuPipeline;
        VuHandle2<uint32>  materialData;

        void init(VuDevice* vuDevice, VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule, VkRenderPass renderPass);

        //void recompile();

        void uninit();

        void bindPipeline(const VkCommandBuffer& commandBuffer) const;


        GPU_PBR_MaterialData* getMaterialData();
    };
}
