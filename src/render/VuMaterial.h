#pragma once
#include "VuGraphicsPipeline.h"
#include "Common.h"
#include "Mesh.h"
#include "VuTexture.h"


struct VuMaterial {

    std::vector<VkDescriptorSet> imageDescriptorSets;
    VuGraphicsPipeline vuPipeline;
    VuPipelineLayout vuPipelineLayout;

    VuTexture* texture0;

    void Dispose() {
        vuPipeline.Dispose();
    }

    void Init(
        const VkDescriptorSetLayout& setLayout,
        const VuPipelineLayout& pipelineLayout,
        const VkShaderModule& vertexShaderModule,
        const VkShaderModule& fragmentShaderModule,
        const VkRenderPass& renderPass,
        VuTexture* texture

    ) {

        texture0 = texture;
        vuPipelineLayout = pipelineLayout;

        auto bindings = Mesh::getBindingDescription();
        auto attribs = Mesh::getAttributeDescriptions();
        vuPipeline.CreateGraphicsPipeline(
            pipelineLayout.pipelineLayout,
            vertexShaderModule,
            fragmentShaderModule,
            bindings,
            attribs,
            renderPass
        );
        createDescSet(setLayout);
    }

    void PushConstants(VkCommandBuffer& commandBuffer, VkShaderStageFlags stage,
                                   uint32_t offset, uint32_t size, const void* pValues) const {

        vkCmdPushConstants(commandBuffer, vuPipelineLayout.pipelineLayout, stage, offset, size, pValues);
    }

    void bindFrameConstants(const VkCommandBuffer& commandBuffer, uint32 currentFrame) const {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipelineLayout.pipelineLayout,
                                0, 1, &Vu::frameConstantDescriptorSets[currentFrame], 0, nullptr);
    }

    void bind(const VkCommandBuffer& commandBuffer, uint32 currentFrame) const {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.Pipeline);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipelineLayout.pipelineLayout,
                                1, 1, &imageDescriptorSets[currentFrame], 0, nullptr);

        // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, debugPipeline.PipelineLayout,
        //                         1, 1, &ImageDescriptorSets[currentFrame], 0, nullptr);
    }

private:
    void createDescSet(const VkDescriptorSetLayout& setLayout) {
        //image
        std::vector imageLayouts(Vu::MAX_FRAMES_IN_FLIGHT, setLayout);
        VkDescriptorSetAllocateInfo imageSetsAllocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = Vu::DescriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(Vu::MAX_FRAMES_IN_FLIGHT),
            .pSetLayouts = imageLayouts.data(),
        };

        imageDescriptorSets.resize(Vu::MAX_FRAMES_IN_FLIGHT);
        VK_CHECK(vkAllocateDescriptorSets(Vu::Device, &imageSetsAllocInfo, imageDescriptorSets.data()));


        //image
        for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.sampler = texture0->textureImageSampler;
            imageInfo.imageView = texture0->textureImageView;

            VkWriteDescriptorSet imageDescWrite{};
            imageDescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            imageDescWrite.dstSet = imageDescriptorSets[i];
            imageDescWrite.dstBinding = 0;
            imageDescWrite.dstArrayElement = 0;
            imageDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            imageDescWrite.descriptorCount = 1;
            imageDescWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(Vu::Device, 1, &imageDescWrite, 0, nullptr);
        }
    }


};
