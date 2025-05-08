#pragma once

#include "10_Core/Common.h"

#include "VuUtils.h"

namespace Vu
{
    struct VuRenderPass
    {
        vk::Device     device;
        vk::Format     colorFormat;
        vk::Format     depthStencilFormat;
        vk::Format     normalFormat;
        vk::RenderPass renderPass;

        std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates;

        void initAsGBufferPass(vk::Device device,
                               vk::Format colorFormat,
                               vk::Format normalFormat,
                               vk::Format aoRoughMetalFormat,
                               vk::Format depthStencilFormat
        )
        {
            this->device             = device;
            this->colorFormat        = colorFormat;
            this->normalFormat       = normalFormat;
            this->depthStencilFormat = depthStencilFormat;

            vk::AttachmentDescription colorAttachment{
                .format = colorFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            vk::AttachmentDescription normalAttachment{
                .format = normalFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };

            vk::AttachmentDescription armAttachment{
                .format = aoRoughMetalFormat,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };


            vk::AttachmentDescription depthAttachment{};
            depthAttachment.format         = depthStencilFormat;
            depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
            depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            std::array attachments = {colorAttachment, normalAttachment, armAttachment, depthAttachment};

            std::array<vk::AttachmentReference, 3> colorRefs = {
                {
                    {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                    {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                    {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}
                }
            };

            vk::AttachmentReference depthRef{
                .attachment = colorRefs.size(),
                .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };

            vk::SubpassDescription subpass{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount = colorRefs.size(),
                .pColorAttachments = colorRefs.data(),
                .pDepthStencilAttachment = &depthRef
            };

            vk::SubpassDependency dependency{};
            dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass      = 0;
            dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask   = 0;
            dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dependency.dependencyFlags = 0;


            vk::RenderPassCreateInfo renderPassInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &dependency
            };

            vk::Check(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
            Utils::giveDebugName(device, VK_OBJECT_TYPE_RENDER_PASS, renderPass, "GBuffer Render Pass");

            colorBlendAttachmentStates.resize(3);
            for (auto& blendAttachment : colorBlendAttachmentStates)
            {
                blendAttachment.blendEnable    = VK_FALSE; // No blending in GBuffer
                blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
        }


        void initAsLightningPass(vk::Device device, vk::Format colorFormat)
        {
            this->device      = device;
            this->colorFormat = colorFormat;

            vk::AttachmentDescription colorAttachment{};
            colorAttachment.format         = colorFormat;
            colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


            vk::AttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            vk::SubpassDescription subpass{};
            subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments    = &colorAttachmentRef;

            vk::SubpassDependency dependency{};
            dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass      = 0;
            dependency.srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.srcAccessMask   = 0;
            dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency.dependencyFlags = 0;

            std::array attachments = {colorAttachment};

            vk::RenderPassCreateInfo renderPassInfo{
                .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount = static_cast<uint32_t>(attachments.size()),
                .pAttachments = attachments.data(),
                .subpassCount = 1,
                .pSubpasses = &subpass,
                .dependencyCount = 1,
                .pDependencies = &dependency
            };

            vk::Check(vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass));
            Utils::giveDebugName(device, VK_OBJECT_TYPE_RENDER_PASS, renderPass, "Lightning Render Pass");

            colorBlendAttachmentStates.resize(1);
            for (auto& blendAttachment : colorBlendAttachmentStates)
            {
                blendAttachment.blendEnable    = VK_FALSE; // No blending in GBuffer
                blendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                                 VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
        }

        void uninit()
        {
            vkDestroyRenderPass(device, renderPass, nullptr);
        }
    };
}
