#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "01_InnerCore/TypeDefs.h" // for u32orNull

namespace Vu {

struct VuDevice;

struct VuSamplerCreateInfo {
  float                  maxAnisotropy = {16.0f};
  vk::SamplerAddressMode addressMode   = {vk::SamplerAddressMode::eRepeat};
};

struct VuSampler {
  const std::shared_ptr<VuDevice>& vuDevice       = {};
  vk::raii::Sampler                sampler        = {nullptr};
  VuSamplerCreateInfo              lastCreateInfo = {};
  u32orNull                        bindlessIndex  = {};

  static std::expected<VuSampler, vk::Result>
  make(const std::shared_ptr<VuDevice>& vuDevice, const VuSamplerCreateInfo& createInfo);

  VuSampler() = default;

private:
  VuSampler(const std::shared_ptr<VuDevice>& vuDevice, const VuSamplerCreateInfo& createInfo);
};
} // namespace Vu
