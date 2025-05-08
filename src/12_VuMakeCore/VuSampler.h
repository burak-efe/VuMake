#pragma once

#include <vulkan/vulkan_core.h>     // for vk::SamplerAddressMode, vk::Device
#include "08_LangUtils/TypeDefs.h"  // for u32orNull

namespace Vu
{
    struct VuSamplerCreateInfo
    {
        float                maxAnisotropy = 16.0f;
        vk::SamplerAddressMode addressMode   = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    };

    struct VuSampler
    {
        vk::Device            device;
        VuSamplerCreateInfo lastCreateInfo;
        vk::Sampler           vkSampler;
        u32orNull           bindlessIndex = 0;

        void init(const vk::Device device, const VuSamplerCreateInfo& createInfo);

        void uninit() const;
    };
}
