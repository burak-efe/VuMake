#pragma once
#include "VuGraphicsPipeline.h"
#include "Common.h"
#include "Mesh.h"
#include "VuTexture.h"

struct VuMaterial {
    VuGraphicsPipeline vuPipeline;

    void Dispose() {
        vuPipeline.Dispose();
    }

    void Init(
        const VkShaderModule& vertexShaderModule,
        const VkShaderModule& fragmentShaderModule,
        const VkRenderPass& renderPass
    ) {
        auto bindings = Mesh::getBindingDescription();
        auto attribs = Mesh::getAttributeDescriptions();
        vuPipeline.CreateGraphicsPipeline(
            Vu::globalPipelineLayout,
            vertexShaderModule,
            fragmentShaderModule,
            bindings,
            attribs,
            renderPass
        );
    }

    void PushConstants(VkCommandBuffer& commandBuffer, VuPushConstant pushConstant) const {
        vkCmdPushConstants(commandBuffer, Vu::globalPipelineLayout, VK_SHADER_STAGE_ALL,
            0, sizeof(VuPushConstant), &pushConstant);
    }

    void bindFrameConstants(const VkCommandBuffer& commandBuffer, uint32 currentFrame) const {
        //frame const
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            Vu::globalPipelineLayout,
            0,
            1,
            &Vu::frameConstantDescriptorSets[currentFrame],
            0,
            nullptr
        );

        //global
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            Vu::globalPipelineLayout,
            1,
            1,
            &Vu::globalDescriptorSets[currentFrame],
            0,
            nullptr
        );

    }

    void bindPipeline(const VkCommandBuffer& commandBuffer) const {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.Pipeline);
    }
};
