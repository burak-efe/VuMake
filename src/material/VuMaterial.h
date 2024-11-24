#pragma once

#include "Common.h"
#include "VuGraphicsPipeline.h"
#include "VuMesh.h"
#include "VuTypes.h"
#include "VuMaterialDataPool.h"


namespace Vu {
    struct VuMaterialCreateInfo {
        VkShaderModule& vertexShaderModule;
        VkShaderModule& fragmentShaderModule;
        VkRenderPass& renderPass;
    };

    struct VuMaterial {
        VuGraphicsPipeline vuPipeline;
        PBRMaterialData* pbrMaterialData;

        void init(const VuMaterialCreateInfo& createInfo) {
            auto bindings = VuMesh::getBindingDescription();
            auto attribs = VuMesh::getAttributeDescriptions();
            vuPipeline.CreateGraphicsPipeline(
                ctx::globalPipelineLayout,
                createInfo.vertexShaderModule,
                createInfo.fragmentShaderModule,
                bindings,
                attribs,
                createInfo.renderPass
            );

            pbrMaterialData = ctx::materialDataPool.allocMaterialData();
        }

        void uninit() {
            vuPipeline.Dispose();
            ctx::materialDataPool.freeMaterialData(pbrMaterialData);
        }

        void bindPipeline(const VkCommandBuffer& commandBuffer) const {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
        }
    };
}
