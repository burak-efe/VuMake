#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "01_InnerCore/TypeDefs.h" // for u32orNull

namespace Vu {
struct VuSamplerCreateInfo {
  float                  maxAnisotropy = {16.0f};
  vk::SamplerAddressMode addressMode   = {vk::SamplerAddressMode::eRepeat};
};

struct VuSampler {
  vk::raii::Sampler   sampler        = {nullptr};
  VuSamplerCreateInfo lastCreateInfo = {};
  u32orNull           bindlessIndex  = {};

  void
  init(const vk::raii::Device& device, const VuSamplerCreateInfo& createInfo);

  // void uninit() const;
};
} // namespace Vu
