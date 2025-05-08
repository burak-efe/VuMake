#include "VuSampler.h"

#include "VuCommon.h"
#include "10_Core/Common.h"

void Vu::VuSampler::init(const vk::Device device,const VuSamplerCreateInfo& createInfo)
{
    this->device = device;
    lastCreateInfo = createInfo;

    vk::SamplerCreateInfo samplerInfo{};
    samplerInfo.sType                   = vk::StructureType::eSamplerCreateInfo;
    samplerInfo.magFilter               = vk::Filter::eLinear;
    samplerInfo.minFilter               = vk::Filter::eLinear;
    samplerInfo.addressModeU            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeV            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.addressModeW            = vk::SamplerAddressMode::eRepeat;
    samplerInfo.anisotropyEnable        = VK_TRUE;
    samplerInfo.maxAnisotropy           = createInfo.maxAnisotropy;
    samplerInfo.borderColor             = vk::BorderColor::eIntOpaqueBlack;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = vk::CompareOp::eAlways;
    samplerInfo.mipmapMode              = vk::SamplerMipmapMode::eLinear;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;


    //todo
    auto res = vkCreateSampler(device, &samplerInfo, nullptr, &vkSampler);
}

void Vu::VuSampler::uninit() const
{
    vkDestroySampler(device, vkSampler, nullptr);
}
