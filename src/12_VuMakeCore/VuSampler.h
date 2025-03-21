#pragma once

#include "10_Core/VuCommon.h"

namespace Vu
{
    struct VuSamplerCreateInfo
    {
        float                maxAnisotropy = 16.0f;
        VkSamplerAddressMode addressMode   = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    };

    struct VuSampler
    {
        VkDevice            device;
        VuSamplerCreateInfo lastCreateInfo;
        VkSampler           vkSampler;

        void init(const VkDevice device, const VuSamplerCreateInfo& createInfo);

        void uninit() const;
    };
}
