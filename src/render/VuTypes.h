#pragma once

#include <optional>
#include <vector>

#include "Common.h"

namespace Vu {

    struct PBRMaterialData {
        uint32 texture0;
        uint32 texture1;
    };

    struct VuPushConstant {
        float4x4 trs;
        uint64 materiealDataPtr;
    };

    struct VuFrameConst {
        float4x4 view;
        float4x4 proj;
        float3 cameraPos;
        float pad0;
        float3 cameraDir;
        float pad1;
        float time;
        float3 pad2;
        float debugIndex = 1;
        float3 pad3;
    };

    struct QueueFamilyIndices {
        std::optional<uint32> graphicsFamily;
        std::optional<uint32> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }

        static QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface) {
            ZoneScoped;
            //Logic to find graphics queue family
            QueueFamilyIndices indices;

            uint32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;

            for (const auto& queuefamily: queueFamilies) {
                if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport) {
                    indices.presentFamily = i;
                }

                if (indices.isComplete()) {
                    break;
                }

                i++;
            }

            return indices;
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

}
