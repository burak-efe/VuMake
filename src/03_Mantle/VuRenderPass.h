#pragma once

#include "vulkan/vulkan_raii.hpp"

namespace Vu {
struct VuRenderPass {
  vk::raii::RenderPass                               renderPass                 = {nullptr};
  std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = {};

  void initAsGBufferPass(const vk::raii::Device& device,
                         const vk::Format        colorFormat,
                         const vk::Format        normalFormat,
                         const vk::Format        aoRoughMetalFormat,
                         const vk::Format        worldPosFormat,
                         const vk::Format        depthStencilFormat) {

    vk::AttachmentDescription colorAttachment {};
    colorAttachment.format         = colorFormat;
    colorAttachment.samples        = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentDescription normalAttachment {};
    normalAttachment.format         = normalFormat;
    normalAttachment.samples        = vk::SampleCountFlagBits::e1;
    normalAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    normalAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    normalAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    normalAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    normalAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    normalAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentDescription armAttachment {};
    armAttachment.format         = aoRoughMetalFormat;
    armAttachment.samples        = vk::SampleCountFlagBits::e1;
    armAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    armAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    armAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    armAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    armAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    armAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentDescription worldPosAttachment {};
    worldPosAttachment.format         = worldPosFormat;
    worldPosAttachment.samples        = vk::SampleCountFlagBits::e1;
    worldPosAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    worldPosAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    worldPosAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    worldPosAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    worldPosAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    worldPosAttachment.finalLayout    = vk::ImageLayout::eShaderReadOnlyOptimal;

    vk::AttachmentDescription depthAttachment {};
    depthAttachment.format         = depthStencilFormat;
    depthAttachment.samples        = vk::SampleCountFlagBits::e1;
    depthAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    depthAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    depthAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    depthAttachment.stencilStoreOp = vk::AttachmentStoreOp::eStore;
    depthAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    depthAttachment.finalLayout    = vk::ImageLayout::eDepthStencilAttachmentOptimal;

    std::array attachments = {colorAttachment, normalAttachment, armAttachment, worldPosAttachment, depthAttachment};

    std::array<vk::AttachmentReference, 4> colorRefs = {{
        {.attachment = 0, .layout = vk::ImageLayout::eColorAttachmentOptimal},
        {.attachment = 1, .layout = vk::ImageLayout::eColorAttachmentOptimal},
        {.attachment = 2, .layout = vk::ImageLayout::eColorAttachmentOptimal},
        {.attachment = 3, .layout = vk::ImageLayout::eColorAttachmentOptimal},
    }};

    vk::AttachmentReference depthRef {.attachment = attachments.size() - 1,
                                      .layout     = vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::SubpassDescription subpass {.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics,
                                    .colorAttachmentCount    = colorRefs.size(),
                                    .pColorAttachments       = colorRefs.data(),
                                    .pDepthStencilAttachment = &depthRef};

    vk::SubpassDependency dependency;
    dependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass   = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependency.dstStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcAccessMask = {};
    dependency.dstAccessMask =
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    dependency.dependencyFlags = {};

    vk::RenderPassCreateInfo renderPassInfo {.attachmentCount = static_cast<uint32_t>(attachments.size()),
                                             .pAttachments    = attachments.data(),
                                             .subpassCount    = 1,
                                             .pSubpasses      = &subpass,
                                             .dependencyCount = 1,
                                             .pDependencies   = &dependency};

    auto renderPassOrErr = device.createRenderPass(renderPassInfo);
    // todo
    throw_if_unexpected(renderPassOrErr);
    this->renderPass = std::move(renderPassOrErr.value());

    colorBlendAttachmentStates.resize(attachments.size() - 1);
    for (auto& blendAttachment : colorBlendAttachmentStates) {
      blendAttachment.blendEnable    = VK_FALSE; // No blending in GBuffer
      blendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                       vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    }
  }

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void initAsLightningPass(const vk::raii::Device& device, vk::Format colorFormat) {

    vk::AttachmentDescription colorAttachment {};
    colorAttachment.format         = colorFormat;
    colorAttachment.samples        = vk::SampleCountFlagBits::e1;
    colorAttachment.loadOp         = vk::AttachmentLoadOp::eClear;
    colorAttachment.storeOp        = vk::AttachmentStoreOp::eStore;
    colorAttachment.stencilLoadOp  = vk::AttachmentLoadOp::eDontCare;
    colorAttachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
    colorAttachment.initialLayout  = vk::ImageLayout::eUndefined;
    colorAttachment.finalLayout    = vk::ImageLayout::ePresentSrcKHR;

    vk::AttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = vk::ImageLayout::eColorAttachmentOptimal;

    vk::SubpassDescription subpass {};
    subpass.pipelineBindPoint    = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    vk::SubpassDependency dependency {};
    dependency.srcSubpass   = vk::SubpassExternal;
    dependency.dstSubpass   = 0;
    dependency.srcStageMask = vk::PipelineStageFlagBits::eBottomOfPipe;
    dependency.dstStageMask =
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests;
    dependency.srcAccessMask   = {};
    dependency.dstAccessMask   = vk::AccessFlagBits::eColorAttachmentWrite;
    dependency.dependencyFlags = {};

    std::array<vk::AttachmentDescription, 1> attachments = {colorAttachment};

    vk::RenderPassCreateInfo renderPassInfo {};
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    auto renderPassOrErr = device.createRenderPass(renderPassInfo);
    // todo
    throw_if_unexpected(renderPassOrErr);
    this->renderPass = std::move(renderPassOrErr.value());

    colorBlendAttachmentStates.resize(1);
    for (auto& blendAttachment : colorBlendAttachmentStates) {
      blendAttachment.blendEnable    = VK_FALSE; // No blending in GBuffer
      blendAttachment.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                       vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    }
  }
};
} // namespace Vu
