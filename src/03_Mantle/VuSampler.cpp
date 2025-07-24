#include "VuSampler.h"

#include "../02_OuterCore/VuCommon.h"
#include "VuDevice.h"

std::expected<Vu::VuSampler, vk::Result>
Vu::VuSampler::make(const std::shared_ptr<VuDevice>& vuDevice, const VuSamplerCreateInfo& createInfo) {
  try {
    VuSampler outSampler {vuDevice, createInfo};
    return outSampler;

  } catch (vk::Result res) { return std::unexpected {res}; } catch (...) {
    return std::unexpected {vk::Result::eErrorUnknown};
  }
}
Vu::VuSampler::VuSampler(const std::shared_ptr<VuDevice>& vuDevice, const VuSamplerCreateInfo& createInfo)
    : vuDevice {vuDevice} {
  lastCreateInfo = createInfo;

  vk::SamplerCreateInfo samplerInfo {};
  samplerInfo.sType                   = vk::StructureType::eSamplerCreateInfo;
  samplerInfo.magFilter               = vk::Filter::eLinear;
  samplerInfo.minFilter               = vk::Filter::eLinear;
  samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
  samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
  samplerInfo.anisotropyEnable        = vk::True;
  samplerInfo.maxAnisotropy           = createInfo.maxAnisotropy;
  samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
  samplerInfo.unnormalizedCoordinates = vk::False;
  samplerInfo.compareEnable           = vk::False;
  samplerInfo.compareOp               = vk::CompareOp::eAlways;
  samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
  samplerInfo.mipLodBias              = 0.0f;
  samplerInfo.minLod                  = 0.0f;
  samplerInfo.maxLod                  = 0.0f;

  auto samplerOrErr = this->vuDevice->device.createSampler(samplerInfo);
  // todo
  throw_if_unexpected(samplerOrErr);
  sampler = std::move(samplerOrErr.value());
}