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
        GPU_PBR_MaterialData* pbrMaterialData;

        void init(const VuMaterialCreateInfo& createInfo) {
            auto bindings = VuMesh::getBindingDescription();
            auto attribs = VuMesh::getAttributeDescriptions();
            vuPipeline.initGraphicsPipeline(
                ctx::vuDevice->globalPipelineLayout,
                createInfo.vertexShaderModule,
                createInfo.fragmentShaderModule,
                bindings,
                attribs,
                createInfo.renderPass
            );

            pbrMaterialData = VuMaterialDataPool::allocMaterialData();
        }

        void recompile(const VuMaterialCreateInfo& createInfo) {
            vuPipeline.Dispose();

            auto bindings = VuMesh::getBindingDescription();
            auto attribs = VuMesh::getAttributeDescriptions();
            vuPipeline.initGraphicsPipeline(
                ctx::vuDevice->globalPipelineLayout,
                createInfo.vertexShaderModule,
                createInfo.fragmentShaderModule,
                bindings,
                attribs,
                createInfo.renderPass
            );

        }

        void uninit() {
            vuPipeline.Dispose();
            VuMaterialDataPool::freeMaterialData(pbrMaterialData);
        }

        void bindPipeline(const VkCommandBuffer& commandBuffer) const {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
        }
    };
}
