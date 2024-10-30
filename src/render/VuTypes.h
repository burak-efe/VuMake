#pragma once

#include <optional>
#include <vector>

#include "Common.h"
#include <vk_mem_alloc.h>

namespace Vu {
    struct VuPushConstant {
        float4x4 trs;
        uint32_t materiealDataOffset;
    };

    struct AllocatedImage {
        VkImage Image;
        VmaAllocation Allocation;
    };

    struct FrameUBO {
        float4x4 view;
        float4x4 proj;
    };


    struct QueueFamilyIndices {
        std::optional<uint32> graphicsFamily;
        std::optional<uint32> presentFamily;

        bool IsComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
}
