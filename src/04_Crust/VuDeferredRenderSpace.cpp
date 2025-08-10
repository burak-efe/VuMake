#include "VuDeferredRenderSpace.h"

#include <array>
#include <expected>
#include <stdint.h>
#include <utility>

#include "02_OuterCore/VuCommon.h"
#include "03_Mantle/VuDevice.h"
#include "03_Mantle/VuImage.h"
#include "03_Mantle/VuRenderPass.h"
#include "VuRenderer.h"

namespace Vu {

VuDeferredRenderSpace::VuDeferredRenderSpace(std::shared_ptr<VuDevice>     vuDevice,
                                             std::shared_ptr<VkSurfaceKHR> surface) :
    m_vuDevice(vuDevice) {

  auto swpChain       = VuSwapChain::make(vuDevice, surface);
  this->m_vuSwapChain = move_or_THROW(swpChain);
  // Color image handle
  auto colorImgOrrErr = VuImage::make(vuDevice,
                                      VuImageCreateInfo {
                                          .width  = m_vuSwapChain.m_extend2D.width,
                                          .height = m_vuSwapChain.m_extend2D.height,
                                          .format = VK_FORMAT_R8G8B8A8_UNORM,
                                          .usage  = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                          .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                      });
  THROW_if_unexpected(colorImgOrrErr);
  m_colorImage = std::make_shared<VuImage>(std::move(colorImgOrrErr.value()));

  // Normal image handle
  auto normalImgOrrErr = VuImage::make(vuDevice,
                                       VuImageCreateInfo {
                                           .width  = m_vuSwapChain.m_extend2D.width,
                                           .height = m_vuSwapChain.m_extend2D.height,
                                           .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                           .usage  = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                           .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                       });
  THROW_if_unexpected(normalImgOrrErr);
  m_normalImage = std::make_shared<VuImage>(std::move(normalImgOrrErr.value()));

  // Arm image handle
  auto armImgOrrErr = VuImage::make(vuDevice,
                                    VuImageCreateInfo {
                                        .width      = m_vuSwapChain.m_extend2D.width,
                                        .height     = m_vuSwapChain.m_extend2D.height,
                                        .format     = VK_FORMAT_R32G32B32A32_SFLOAT,
                                        .usage      = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    });
  THROW_if_unexpected(armImgOrrErr);
  m_aoRoughMetalImage = std::make_shared<VuImage>(std::move(armImgOrrErr.value()));

  // world space pos image handle
  auto wsPosImageOrErr = VuImage::make(vuDevice,
                                       VuImageCreateInfo {
                                           .width  = m_vuSwapChain.m_extend2D.width,
                                           .height = m_vuSwapChain.m_extend2D.height,
                                           .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                           .usage  = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                           .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                       });
  THROW_if_unexpected(wsPosImageOrErr);
  m_worldSpacePosImage = std::make_shared<VuImage>(std::move(wsPosImageOrErr.value()));

  // Depth-stencil image handle
  auto depthStencilImgOrrErr =
      VuImage::make(vuDevice,
                    VuImageCreateInfo {
                        .width      = m_vuSwapChain.m_extend2D.width,
                        .height     = m_vuSwapChain.m_extend2D.height,
                        .format     = VK_FORMAT_D32_SFLOAT,
                        .usage      = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    });
  THROW_if_unexpected(depthStencilImgOrrErr);
  m_depthStencilImage = std::make_shared<VuImage>(std::move(depthStencilImgOrrErr.value()));

  m_gBufferPass   = std::make_shared<VuRenderPass>(nullptr);
  m_lightningPass = std::make_shared<VuRenderPass>(nullptr);

  m_gBufferPass->initAsGBufferPass(vuDevice,
                                   m_colorImage->m_lastCreateInfo.format,
                                   m_normalImage->m_lastCreateInfo.format,
                                   m_aoRoughMetalImage->m_lastCreateInfo.format,
                                   m_worldSpacePosImage->m_lastCreateInfo.format,
                                   m_depthStencilImage->m_lastCreateInfo.format);
  m_lightningPass->initAsLightningPass(vuDevice, m_vuSwapChain.m_imageFormat);

  createFramebuffers(*vuDevice);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
VuDeferredRenderSpace::registerImagesToBindless(VuRenderer& vuRenderer) {
  vuRenderer.registerToBindless(*m_colorImage);
  vuRenderer.registerToBindless(*m_normalImage);
  vuRenderer.registerToBindless(*m_aoRoughMetalImage);
  vuRenderer.registerToBindless(*m_worldSpacePosImage);
  vuRenderer.registerToBindless(*m_depthStencilImage);
  m_lightningPassMaterialData.colorTexture         = m_colorImage->m_bindlessIndex.value_or_THROW();
  m_lightningPassMaterialData.normalTexture        = m_normalImage->m_bindlessIndex.value_or_THROW();
  m_lightningPassMaterialData.aoRoughMetalTexture  = m_aoRoughMetalImage->m_bindlessIndex.value_or_THROW();
  m_lightningPassMaterialData.worldSpacePosTexture = m_worldSpacePosImage->m_bindlessIndex.value_or_THROW();
  m_lightningPassMaterialData.depthTexture         = m_depthStencilImage->m_bindlessIndex.value_or_THROW();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
VuDeferredRenderSpace::createFramebuffers(const VuDevice& vuDevice) {
  m_gPassFrameBuffers.clear();
  m_gPassFrameBuffers.resize(m_vuSwapChain.m_imageViews.size());
  for (size_t i = 0; i < m_vuSwapChain.m_imageViews.size(); i++) {
    std::array<VkImageView, 5> attachments = {
        m_colorImage->m_imageView,
        m_normalImage->m_imageView,
        m_aoRoughMetalImage->m_imageView,
        m_worldSpacePosImage->m_imageView,
        m_depthStencilImage->m_imageView,
    };
    VkFramebufferCreateInfo framebufferInfo {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferInfo.renderPass      = m_gBufferPass->m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = m_vuSwapChain.m_extend2D.width;
    framebufferInfo.height          = m_vuSwapChain.m_extend2D.height;
    framebufferInfo.layers          = 1;

    VkResult frameBufferRes =
        vkCreateFramebuffer(vuDevice.m_device, &framebufferInfo, NO_ALLOC_CALLBACK, &m_gPassFrameBuffers[i]);
    THROW_if_fail(frameBufferRes);
  }

  m_lightningPassFrameBuffers.clear();
  m_lightningPassFrameBuffers.resize(m_vuSwapChain.m_imageViews.size());
  for (size_t i = 0; i < m_vuSwapChain.m_imageViews.size(); i++) {
    std::array<VkImageView, 1> attachments = {
        m_vuSwapChain.m_imageViews[i],
    };
    VkFramebufferCreateInfo framebufferInfo {.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    framebufferInfo.renderPass      = m_lightningPass->m_renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments    = attachments.data();
    framebufferInfo.width           = m_vuSwapChain.m_extend2D.width;
    framebufferInfo.height          = m_vuSwapChain.m_extend2D.height;
    framebufferInfo.layers          = 1;

    VkResult frameBufferRes =
        vkCreateFramebuffer(vuDevice.m_device, &framebufferInfo, NO_ALLOC_CALLBACK, &m_lightningPassFrameBuffers[i]);
    THROW_if_fail(frameBufferRes);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
VuDeferredRenderSpace::beginLightningPass(const VkCommandBuffer& commandBuffer, const u32 frameIndex) const {
  std::array<VkClearValue, 1> clearValues {};
  clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};

  VkRenderPassBeginInfo renderPassInfo {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  renderPassInfo.renderPass        = m_lightningPass->m_renderPass;
  renderPassInfo.framebuffer       = m_lightningPassFrameBuffers[frameIndex];
  renderPassInfo.renderArea.offset = VkOffset2D {0, 0};
  renderPassInfo.renderArea.extent = m_vuSwapChain.m_extend2D;
  renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues      = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
VuDeferredRenderSpace::beginGBufferPass(const VkCommandBuffer& commandBuffer, const u32 frameIndex) const {
  std::array<VkClearValue, 5> clearValues {
      {{.color = {0, 0, 0, 1}},
       {.color = {0, 0, 0, 1}},
       {.color = {0, 0, 0, 1}},
       {.color = {0, 0, 0, 1}},
       {.depthStencil = {.depth = 1}}},
  };

  VkRenderPassBeginInfo renderPassInfo {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
  renderPassInfo.renderPass        = m_gBufferPass->m_renderPass;
  renderPassInfo.framebuffer       = m_gPassFrameBuffers[frameIndex];
  renderPassInfo.renderArea.offset = VkOffset2D {0, 0};
  renderPassInfo.renderArea.extent = m_vuSwapChain.m_extend2D;
  renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues      = clearValues.data();

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
} // namespace Vu