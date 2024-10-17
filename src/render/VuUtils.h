#pragma once

#include "Common.h"
#include <optional>
#include <vector>
#include <fstream>
#include <stacktrace>
#include <iostream>


#include "Vu.h"
#include "vulkan/vk_enum_string_helper.h"

__declspec(noinline) static void VK_CHECK(VkResult res) {
    if (res != VK_SUCCESS) {
        // creating and initializing stacktrace object
        auto st = std::stacktrace::current();
        // printing stacktrace
        std::cout << "[ERROR] VkResult is " << string_VkResult(res) << " at "
                << st[1].source_file() << " line " << st[1].source_line() << "\n";
    }
}

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



namespace Vu {

     inline void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }


    inline std::vector<char> ReadFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }


    inline VkImageCreateInfo CreateImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;

        info.imageType = VK_IMAGE_TYPE_2D;

        info.format = format;
        info.extent = extent;

        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usageFlags;

        return info;
    }

    inline VkImageViewCreateInfo CreateImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
        //build a image-view for the depth image to use for rendering
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = nullptr;

        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = image;
        info.format = format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = aspectFlags;

        return info;
    }

    inline QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
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

            if (indices.IsComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    inline uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(Vu::PhysicalDevice, &memProperties);
        for (uint32 i = 0; i < memProperties.memoryTypeCount; i++) {
            if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
}
