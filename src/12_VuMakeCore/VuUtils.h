#pragma once

#include "10_Core/VuCommon.h"

namespace Vu::Utils
{
    VkImageCreateInfo fillImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent);

    VkImageViewCreateInfo fillImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags);

    uint32 findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties,
                               uint32                                  typeFilter,
                               VkMemoryPropertyFlags                   requiredProperties);


    void giveDebugName(const VkDevice device, const VkObjectType objType, const void* objHandle, const char* debugName);


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
