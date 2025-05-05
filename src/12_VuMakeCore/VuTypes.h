#pragma once

#include <functional>
#include <stack>
#include <optional>

#include "10_Core/FixedString.h"
#include "10_Core/VuCommon.h"
#include "10_Core/math/VuFloat4.h"
#include "10_Core/math/VuFloat4x4.h"


namespace Vu
{
    using VuName = FixedString64;

    struct GPU_Mesh
    {
        u32 vertexBufferHandle;
        u32 vertexCount;
        u32 meshFlags;
    };


    struct GPU_PBR_MaterialData
    {
        u32 texture0;
        u32 texture1;
        u32 texture2;
        u32 texture3;

        byte padding[48];
    };

    static_assert(sizeof(GPU_PBR_MaterialData) == 64);


    struct GPU_PushConstant
    {
        mat4x4 trs;
        u32   materialDataIndex;
        GPU_Mesh mesh;
    };

    struct GPU_FrameConst
    {
        mat4x4 view;
        mat4x4 proj;
        mat4x4 inverseView;
        mat4x4 inverseProj;
        vec4   cameraPos;
        vec4   cameraDir;
        float    time;
        float    debugIndex;
    };


    struct VuDisposeStack
    {
        std::stack<std::function<void()>> disposeStack;

        void push(const std::function<void()>& func)
        {
            disposeStack.push(func);
        }

        void disposeAll()
        {
            while (!disposeStack.empty())
            {
                std::function<void()> disposeFunc = disposeStack.top();
                disposeFunc();
                disposeStack.pop();
            }
        }
    };

    struct QueueFamilyIndices
    {
        std::optional<u32> graphicsFamily;
        std::optional<u32> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }

        static QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice device, const VkSurfaceKHR surface)
        {
            ZoneScoped;
            //Logic to find graphics queue family
            QueueFamilyIndices indices;

            u32 queueFamilyCount = 0;
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

            std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

            int i = 0;

            for (const auto& queue_family : queueFamilies)
            {
                if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                {
                    indices.graphicsFamily = i;
                }

                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

                if (presentSupport)
                {
                    indices.presentFamily = i;
                }

                if (indices.isComplete())
                {
                    break;
                }

                i++;
            }

            return indices;
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR        capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR>   presentModes;

        static SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface)
        {
            SwapChainSupportDetails details;
            vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

            u32 formatCount;
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);

            if (formatCount != 0)
            {
                details.formats.resize(formatCount);
                vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());
            }

            u32 presentModeCount;
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

            if (presentModeCount != 0)
            {
                details.presentModes.resize(presentModeCount);
                vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount,
                                                          details.presentModes.data());
            }

            return details;
        }
    };
}
