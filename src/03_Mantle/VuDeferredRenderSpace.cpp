//
// Created by User0 on 5/11/25.
//

#include "VuDeferredRenderSpace.h"

#include "04_Crust/VuRenderer.h"

namespace Vu {

VuDeferredRenderSpace::VuDeferredRenderSpace(const std::shared_ptr<VuDevice>&             vuDevice,
                                             const std::shared_ptr<vk::raii::SurfaceKHR>& surface)
    : vuDevice {vuDevice}, vuSwapChain(vuDevice, surface) {

  // Color image handle
  auto colorImgOrrErr =
      VuImage::make(vuDevice, VuImageCreateInfo {
                                  .width  = vuSwapChain.extend2D.width,
                                  .height = vuSwapChain.extend2D.height,
                                  .format = vk::Format::eR8G8B8A8Unorm,
                                  .usage  = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
                                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                              });
  throw_if_unexpected(colorImgOrrErr);
  colorImage = std::make_shared<VuImage>(std::move(colorImgOrrErr.value()));

  // Normal image handle
  auto normalImgOrrErr =
      VuImage::make(vuDevice, VuImageCreateInfo {
                                  .width  = vuSwapChain.extend2D.width,
                                  .height = vuSwapChain.extend2D.height,
                                  .format = vk::Format::eR32G32B32A32Sfloat,
                                  .usage  = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
                                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                              });
  throw_if_unexpected(normalImgOrrErr);
  normalImage = std::make_shared<VuImage>(std::move(normalImgOrrErr.value()));

  // Arm image handle
  auto armImgOrrErr =
      VuImage::make(vuDevice, VuImageCreateInfo {
                                  .width  = vuSwapChain.extend2D.width,
                                  .height = vuSwapChain.extend2D.height,
                                  .format = vk::Format::eR32G32B32A32Sfloat,
                                  .usage  = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
                                  .aspectMask = vk::ImageAspectFlagBits::eColor,
                              });
  throw_if_unexpected(armImgOrrErr);
  aoRoughMetalImage = std::make_shared<VuImage>(std::move(armImgOrrErr.value()));

  // Depth-stencil image handle
  auto depthStencilImgOrrErr = VuImage::make(
      vuDevice, VuImageCreateInfo {
                    .width      = vuSwapChain.extend2D.width,
                    .height     = vuSwapChain.extend2D.height,
                    .format     = vk::Format::eD32Sfloat,
                    .usage      = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eDepthStencilAttachment,
                    .aspectMask = vk::ImageAspectFlagBits::eDepth,
                });
  throw_if_unexpected(depthStencilImgOrrErr);
  depthStencilImage = std::make_shared<VuImage>(std::move(depthStencilImgOrrErr.value()));

  gBufferPass   = std::make_shared<VuRenderPass>();
  lightningPass = std::make_shared<VuRenderPass>();

  gBufferPass->initAsGBufferPass(vuDevice->device, colorImage->lastCreateInfo.format,
                                 normalImage->lastCreateInfo.format, aoRoughMetalImage->lastCreateInfo.format,
                                 depthStencilImage->lastCreateInfo.format);
  lightningPass->initAsLightningPass(vuDevice->device, vuSwapChain.imageFormat);

  createFramebuffers(*vuDevice);
}
void
VuDeferredRenderSpace::registerImagesToBindless(VuRenderer& vuRenderer) {
  vuRenderer.registerToBindless(*colorImage);
  vuRenderer.registerToBindless(*normalImage);
  vuRenderer.registerToBindless(*aoRoughMetalImage);
  vuRenderer.registerToBindless(*depthStencilImage);
  lightningPassMaterialData.texture0 = colorImage->bindlessIndex;
  lightningPassMaterialData.texture1 = normalImage->bindlessIndex;
  lightningPassMaterialData.texture2 = aoRoughMetalImage->bindlessIndex;
  lightningPassMaterialData.texture3 = depthStencilImage->bindlessIndex;
}
void
VuDeferredRenderSpace::createFramebuffers(const VuDevice& vuDevice) {
  gPassFrameBuffers.clear();
  for (size_t i = 0; i < vuSwapChain.imageViews.size(); i++) {
    std::array<vk::ImageView, 4> attachments = {
        colorImage->imageView,
        normalImage->imageView,
        aoRoughMetalImage->imageView,
        depthStencilImage->imageView,
    };
    vk::FramebufferCreateInfo framebufferInfo {};
    framebufferInfo.renderPass      = gBufferPass->renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = vuSwapChain.extend2D.width;
    framebufferInfo.height          = vuSwapChain.extend2D.height;
    framebufferInfo.layers          = 1;

    auto frameBufferOrErr = vuDevice.device.createFramebuffer(framebufferInfo);
    // todo
    throw_if_unexpected(frameBufferOrErr);
    gPassFrameBuffers.emplace_back(std::move(frameBufferOrErr.value()));
  }

  lightningPassFrameBuffers.clear();
  for (size_t i = 0; i < vuSwapChain.imageViews.size(); i++) {
    std::array<vk::ImageView, 1> attachments = {
        vuSwapChain.imageViews[i],
    };
    vk::FramebufferCreateInfo framebufferInfo {};
    framebufferInfo.renderPass      = lightningPass->renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = vuSwapChain.extend2D.width;
    framebufferInfo.height          = vuSwapChain.extend2D.height;
    framebufferInfo.layers          = 1;

    auto frameBufferOrErr = vuDevice.device.createFramebuffer(framebufferInfo);
    // todo
    throw_if_unexpected(frameBufferOrErr);
    lightningPassFrameBuffers.emplace_back(std::move(frameBufferOrErr.value()));
  }
}
void
VuDeferredRenderSpace::beginLightningPass(const vk::CommandBuffer& commandBuffer, const u32 frameIndex) const {
  std::array<vk::ClearValue, 1> clearValues {};
  clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});

  vk::RenderPassBeginInfo renderPassInfo {};
  renderPassInfo.renderPass        = lightningPass->renderPass;
  renderPassInfo.framebuffer       = lightningPassFrameBuffers[frameIndex];
  renderPassInfo.renderArea.offset = vk::Offset2D {0, 0};
  renderPassInfo.renderArea.extent = vuSwapChain.extend2D;
  renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues      = clearValues.data();

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}
void
VuDeferredRenderSpace::beginGBufferPass(const vk::CommandBuffer& commandBuffer, const u32 frameIndex) const {
  std::array<vk::ClearValue, 4> clearValues {};
  clearValues[0].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
  clearValues[1].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
  clearValues[3].color.setFloat32({0.0f, 0.0f, 0.0f, 1.0f});
  clearValues[3].depthStencil.setDepth(1.0f);

  vk::RenderPassBeginInfo renderPassInfo {};
  renderPassInfo.renderPass        = gBufferPass->renderPass;
  renderPassInfo.framebuffer       = gPassFrameBuffers[frameIndex];
  renderPassInfo.renderArea.offset = vk::Offset2D {0, 0};
  renderPassInfo.renderArea.extent = vuSwapChain.extend2D;
  renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues      = clearValues.data();

  commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
}
} // namespace Vu