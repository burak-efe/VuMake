#pragma once

#include "01_InnerCore/TypeDefs.h" // for u32orNull
#include "01_InnerCore/zero_optional.h"
#include "02_OuterCore/VuCommon.h"
#include "VuDevice.h"

namespace Vu {

struct VuDevice;

struct VuSamplerCreateInfo {
  float                maxAnisotropy {16.0f};
  VkSamplerAddressMode addressMode {VK_SAMPLER_ADDRESS_MODE_REPEAT};
};
// ######################################################################################################################

struct VuSampler {
  std::shared_ptr<VuDevice> m_vuDevice {nullptr};
  VkSampler                 m_sampler {nullptr};
  zero_optional<u32>        m_bindlessIndex {};

  //--------------------------------------------------------------------------------------------------------------------
  VuSampler()                 = default;
  VuSampler(const VuSampler&) = delete;
  VuSampler&
  operator=(const VuSampler&) = delete;

  VuSampler(VuSampler&& other) noexcept :
      m_vuDevice(std::move(other.m_vuDevice)),
      m_sampler(other.m_sampler),
      m_bindlessIndex(other.m_bindlessIndex) {
    other.m_sampler       = VK_NULL_HANDLE;
    other.m_bindlessIndex = zero_optional<u32> {};
  }

  VuSampler&
  operator=(VuSampler&& other) noexcept {
    if (this != &other) {
      cleanup();
      m_vuDevice      = std::move(other.m_vuDevice);
      m_sampler       = other.m_sampler;
      m_bindlessIndex = other.m_bindlessIndex;

      other.m_sampler       = VK_NULL_HANDLE;
      other.m_bindlessIndex = zero_optional<u32> {};
    }
    return *this;
  }

  ~VuSampler() { cleanup(); }

  SETUP_EXPECTED_WRAPPER(VuSampler,
                         (std::shared_ptr<VuDevice> vuDevice, const VuSamplerCreateInfo& createInfo),
                         (vuDevice, createInfo))
private:
  void
  cleanup() {
    if (m_sampler != VK_NULL_HANDLE) {
      vkDestroySampler(m_vuDevice->m_device, m_sampler, nullptr);
      m_sampler = VK_NULL_HANDLE;
    }
    m_vuDevice.reset();
  }
  //--------------------------------------------------------------------------------------------------------------------

  VuSampler(std::shared_ptr<VuDevice> vuDevice, const VuSamplerCreateInfo& createInfo);
};
} // namespace Vu
