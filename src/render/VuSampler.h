#pragma once
#include "Common.h"

namespace Vu {

    struct VuSamplerCreateInfo {
        VkDevice device;
        VkPhysicalDevice physicalDevice;
    };

    struct VuSampler {
        VuSamplerCreateInfo lastCreateInfo;
        VkSampler vkSampler;

        void init(const VuSamplerCreateInfo& createInfo);

        void uninit() const;
    };
}
