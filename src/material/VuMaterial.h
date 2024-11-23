#pragma once

#include "Common.h"
#include "VuGraphicsPipeline.h"
#include "VuMesh.h"
#include "VuTypes.h"
#include "VuMaterialDataPool.h"



namespace Vu {
    struct VuMaterial {
        VuGraphicsPipeline vuPipeline;
        PBRMaterialData* pbrMaterialData;


        void dispose() {
            vuPipeline.Dispose();
            ctx::materialDataPool.freeMaterialData(pbrMaterialData);
        }

        void init(
            const VkShaderModule& vertexShaderModule,
            const VkShaderModule& fragmentShaderModule,
            const VkRenderPass& renderPass
        ) {
            auto bindings = VuMesh::getBindingDescription();
            auto attribs = VuMesh::getAttributeDescriptions();
            vuPipeline.CreateGraphicsPipeline(
                ctx::globalPipelineLayout,
                vertexShaderModule,
                fragmentShaderModule,
                bindings,
                attribs,
                renderPass
            );

            pbrMaterialData = ctx::materialDataPool.allocMaterialData();
        }

        void bindPipeline(const VkCommandBuffer& commandBuffer) const {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
        }


    private:

    };
}
