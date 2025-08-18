#pragma once

#include <vector>

#include "02_OuterCore/VuCommon.h"
#include "VuDevice.h"
namespace Vu {
struct VuDevice;
struct VuRenderPass {
  std::shared_ptr<VuDevice>                        m_vuDevice {nullptr};
  VkRenderPass                                     m_renderPass {nullptr};
  std::vector<VkPipelineColorBlendAttachmentState> m_colorBlendAttachmentStates {};

  void
  initAsGBufferPass(std::shared_ptr<VuDevice> vuDevice,
                    VkFormat                  colorFormat,
                    VkFormat                  normalFormat,
                    VkFormat                  aoRoughMetalFormat,
                    VkFormat                  worldPosFormat,
                    VkFormat                  depthStencilFormat);

  void
  initAsLightningPass(std::shared_ptr<VuDevice> vuDevice, VkFormat colorFormat);

  //--------------------------------------------------------------------------------------------------------------------
  VuRenderPass(std::nullptr_t) {};
  VuRenderPass()                    = default;
  VuRenderPass(const VuRenderPass&) = delete;
  VuRenderPass&
  operator=(const VuRenderPass&) = delete;

  VuRenderPass(VuRenderPass&& other) noexcept :
      m_vuDevice(std::move(other.m_vuDevice)),
      m_renderPass(other.m_renderPass),
      m_colorBlendAttachmentStates(std::move(other.m_colorBlendAttachmentStates)) {
    other.m_renderPass = VK_NULL_HANDLE;
  }

  VuRenderPass&
  operator=(VuRenderPass&& other) noexcept {
    if (this != &other) {
      cleanup();
      m_vuDevice                   = std::move(other.m_vuDevice);
      m_renderPass                 = other.m_renderPass;
      m_colorBlendAttachmentStates = std::move(other.m_colorBlendAttachmentStates);
      other.m_renderPass           = VK_NULL_HANDLE;
    }
    return *this;
  }

  ~VuRenderPass() { cleanup(); }

private:
  void
  cleanup() {
    if (m_renderPass != VK_NULL_HANDLE) {
      vkDestroyRenderPass(m_vuDevice->m_device, m_renderPass, nullptr);
      m_renderPass = VK_NULL_HANDLE;
      m_vuDevice.reset();
    }
  }
  //--------------------------------------------------------------------------------------------------------------------
};
} // namespace Vu
