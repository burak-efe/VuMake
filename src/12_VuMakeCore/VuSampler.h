#pragma once

#include <vulkan/vulkan_core.h>     // for VkSamplerAddressMode, VkDevice
#include "08_LangUtils/TypeDefs.h"  // for u32orNull

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
        u32orNull           bindlessIndex = 0;

        void init(const VkDevice device, const VuSamplerCreateInfo& createInfo);

        void uninit() const;
    };
}
