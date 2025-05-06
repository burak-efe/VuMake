#pragma once

#include <span>                     // for span

#include <vulkan/vulkan_core.h>     // for VkFormat, VkPhysicalDevice, VkBool32

#include "08_LangUtils/TypeDefs.h"  // for u32

namespace Vu::Utils
{
    VkImageCreateInfo fillImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

    VkImageViewCreateInfo fillImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    u32 findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties,
                               u32                                  typeFilter,
                               VkMemoryPropertyFlags                   requiredProperties);


    void giveDebugName( VkDevice device,  VkObjectType objType, const void* objHandle, const char* debugName);


    VkBool32 debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                           VkDebugUtilsMessageTypeFlagsEXT             messageType,
                           const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                           void*                                       pUserData);

    void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);


    bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> requestedExtensions);

    bool isDeviceSupportExtensions(VkPhysicalDevice       device, VkSurfaceKHR surface,
                                   std::span<const char*> enabledExtensions);

    bool checkValidationLayerSupport(std::span<const char*> validationLayers);
}
