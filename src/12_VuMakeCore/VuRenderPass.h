#pragma once

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

        vk::AttachmentDescription colorAttachment{};
        colorAttachment.format         = colorFormat;
        colorAttachment.samples        = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;

        vk::AttachmentDescription normalAttachment{};
        normalAttachment.format         = normalFormat;
        normalAttachment.samples        = vk::SampleCountFlagBits::e1;
        normalAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        normalAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        normalAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        normalAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        normalAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        normalAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;

        vk::AttachmentDescription armAttachment{};
        armAttachment.format         = aoRoughMetalFormat;
        armAttachment.samples        = vk::SampleCountFlagBits::e1;
        armAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        armAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        armAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        armAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        armAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        armAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;


        vk::AttachmentDescription depthAttachment{};
        depthAttachment.format         = depthStencilFormat;
        depthAttachment.samples        = vk::SampleCountFlagBits::e1;
        depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        depthAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eStore;
        depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        depthAttachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;


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
        dependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass   = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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
        colorAttachment.samples        = vk::SampleCountFlagBits::e1;
        colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
        colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
        colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
        colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
        colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
        colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

        vk::AttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

        vk::SubpassDescription subpass{};
        subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments    = &colorAttachmentRef;

        vk::SubpassDependency dependency{};
        dependency.srcSubpass   = vk::SubpassExternal;
        dependency.dstSubpass   = 0;
        dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
        dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput |
                                  vk::PipelineStageFlagBits::eEarlyFragmentTests;
        dependency.srcAccessMask   = {};
        dependency.dstAccessMask   = vk::AccessFlagBits::eColorAttachmentWrite;
        dependency.dependencyFlags = {};

        std::array<vk::AttachmentDescription, 1> attachments = {colorAttachment};

        vk::RenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType           = vk::StructureType::eRenderPassCreateInfo;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments    = attachments.data();
        renderPassInfo.subpassCount    = 1;
        renderPassInfo.pSubpasses      = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies   = &dependency;


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
