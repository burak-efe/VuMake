#pragma once

#include "03_Mantle/VuImage.h"
#include "03_Mantle/VuRenderPass.h"
#include "03_Mantle/VuSwapChain.h"
#include "InteroptStructs.h"

namespace Vu {
struct VuRenderer;

struct VuDeferredRenderSpace {
  std::shared_ptr<VuRenderer> vuRenderer = {};

  std::shared_ptr<VuDevice>          vuDevice                  = {};
  VuSwapChain                        vuSwapChain               = {};
  std::shared_ptr<VuImage>           colorImage                = {};
  std::shared_ptr<VuImage>           normalImage               = {};
  std::shared_ptr<VuImage>           aoRoughMetalImage         = {};
  std::shared_ptr<VuImage>           worldSpacePosImage        = {};
  std::shared_ptr<VuImage>           depthStencilImage         = {};
  std::vector<vk::raii::Framebuffer> gPassFrameBuffers         = {};
  std::vector<vk::raii::Framebuffer> lightningPassFrameBuffers = {};
  std::shared_ptr<VuRenderPass>      gBufferPass               = {};
  std::shared_ptr<VuRenderPass>      lightningPass             = {};
  MatData_PbrDeferred                lightningPassMaterialData = {};

  VuDeferredRenderSpace() = default;

  VuDeferredRenderSpace(const std::shared_ptr<VuDevice>&             vuDevice,
                        const std::shared_ptr<vk::raii::SurfaceKHR>& surface);

  void registerImagesToBindless(VuRenderer& vuInstance);

  void beginGBufferPass(const vk::CommandBuffer& commandBuffer, u32 frameIndex) const;

  void beginLightningPass(const vk::CommandBuffer& commandBuffer, u32 frameIndex) const;

private:
  void createFramebuffers(const VuDevice& vuDevice);
};

} // namespace Vu
