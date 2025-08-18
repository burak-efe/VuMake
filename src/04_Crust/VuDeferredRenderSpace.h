#pragma once
#include "03_Mantle/VuSwapChain.h"
#include "InteroptStructs.h"

namespace Vu {
struct VuRenderer;
struct VuDevice;
struct VuImage;
struct VuRenderPass;

struct VuDeferredRenderSpace {
  std::shared_ptr<VuRenderer>   m_vuRenderer {};
  std::shared_ptr<VuDevice>     m_vuDevice {};
  std::shared_ptr<VuImage>      m_colorImage {};
  std::shared_ptr<VuImage>      m_normalImage {};
  std::shared_ptr<VuImage>      m_aoRoughMetalImage {};
  std::shared_ptr<VuImage>      m_worldSpacePosImage {};
  std::shared_ptr<VuImage>      m_depthStencilImage {};
  std::vector<VkFramebuffer>    m_gPassFrameBuffers {};
  std::vector<VkFramebuffer>    m_lightningPassFrameBuffers {};
  std::shared_ptr<VuRenderPass> m_gBufferPass {};
  std::shared_ptr<VuRenderPass> m_lightningPass {};
  GPU::MatData_PbrDeferred           m_lightningPassMaterialData {};
  VuSwapChain                   m_vuSwapChain {};

  ~VuDeferredRenderSpace() {
    for (auto frameBuffer : m_gPassFrameBuffers) {

      vkDestroyFramebuffer(m_vuDevice->m_device, frameBuffer, NO_ALLOC_CALLBACK);
    }
    for (auto frameBuffer : m_lightningPassFrameBuffers) {

      vkDestroyFramebuffer(m_vuDevice->m_device, frameBuffer, NO_ALLOC_CALLBACK);
    }
  }

  VuDeferredRenderSpace()                             = default;
  VuDeferredRenderSpace(const VuDeferredRenderSpace&) = delete;
  VuDeferredRenderSpace&
  operator=(const VuDeferredRenderSpace&)        = delete;
  VuDeferredRenderSpace(VuDeferredRenderSpace&&) = default;
  VuDeferredRenderSpace&
  operator=(VuDeferredRenderSpace&&) = default;

  VuDeferredRenderSpace(std::shared_ptr<VuDevice> vuDevice, std::shared_ptr<VuSurface> surface);

  void
  registerImagesToBindless(VuRenderer& vuInstance);

  void
  beginGBufferPass(const VkCommandBuffer& commandBuffer, uint32_t frameIndex) const;

  void
  beginLightningPass(const VkCommandBuffer& commandBuffer, uint32_t frameIndex) const;

private:
  void
  createFramebuffers(const VuDevice& vuDevice);
};

} // namespace Vu
