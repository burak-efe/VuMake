#include "VuSampler.h"

void Vu::VuSampler::init(const vk::raii::Device& device, const VuSamplerCreateInfo& createInfo)
{
    lastCreateInfo = createInfo;

    vk::SamplerCreateInfo samplerInfo{};
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


    auto samplerOrErr = device.createSampler(samplerInfo);
    //todo
    if (!samplerOrErr) { throw std::runtime_error("Failed to create sampler object"); }
    sampler = std::move(samplerOrErr.value());
}