#pragma once

#include "Common.h"

#include "VuGraphicsPipeline.h"
#include "VuTypes.h"



namespace Vu {
    struct VuMaterialCreateInfo {
        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
        VkRenderPass renderPass;
    };

    struct VuMaterial {
        VuGraphicsPipeline vuPipeline;
        GPU_PBR_MaterialData* pbrMaterialData;

        void init(const VuMaterialCreateInfo& createInfo);

        void recompile(const VuMaterialCreateInfo& createInfo);

        void uninit();

        void bindPipeline(const VkCommandBuffer& commandBuffer) const;
    };
}
