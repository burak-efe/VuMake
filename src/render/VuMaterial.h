#pragma once
#include "VuGraphicsPipeline.h"
#include "Common.h"
#include "VuMesh.h"
#include "VuTypes.h"

namespace Vu {

    struct PBRMaterialData {
        uint32 colorMap;
        uint32 normalMap;
    };

    struct VuMaterial {
        VuGraphicsPipeline vuPipeline;

        void dispose() {
            vuPipeline.Dispose();
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
        }

        void pushConstants(VkCommandBuffer& commandBuffer, VuPushConstant pushConstant) const {
            vkCmdPushConstants(commandBuffer, ctx::globalPipelineLayout, VK_SHADER_STAGE_ALL,
                               0, sizeof(VuPushConstant), &pushConstant);
        }

        void bindFrameConstants(const VkCommandBuffer& commandBuffer, uint32 currentFrame) const {
            // //frame const
            // vkCmdBindDescriptorSets(
            //     commandBuffer,
            //     VK_PIPELINE_BIND_POINT_GRAPHICS,
            //     ctx::globalPipelineLayout,
            //     0,
            //     1,
            //     &ctx::frameConstantDescriptorSets[currentFrame],
            //     0,
            //     nullptr
            // );

            //global
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                ctx::globalPipelineLayout,
                0,
                1,
                &ctx::globalDescriptorSets[currentFrame],
                0,
                nullptr
            );

        }

        void bindPipeline(const VkCommandBuffer& commandBuffer) const {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
        }

    private:
    };
}
