#include "VuUtils.h"

#include <iostream>
#include <set>

#include "VuTypes.h"

VkImageCreateInfo Vu::Utils::fillImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent)
{
    VkImageCreateInfo info = {};
    info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.pNext             = nullptr;
    info.imageType         = VK_IMAGE_TYPE_2D;
    info.format            = format;
    info.extent            = extent;
    info.mipLevels         = 1;
    info.arrayLayers       = 1;
    info.samples           = VK_SAMPLE_COUNT_1_BIT;
    info.tiling            = VK_IMAGE_TILING_OPTIMAL;
    info.usage             = usageFlags;
    return info;
}

VkImageViewCreateInfo Vu::Utils::fillImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo info           = {};
    info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.pNext                           = nullptr;
    info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    info.image                           = image;
    info.format                          = format;
    info.subresourceRange.baseMipLevel   = 0;
    info.subresourceRange.levelCount     = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount     = 1;
    info.subresourceRange.aspectMask     = aspectFlags;
    return info;
}

u32 Vu::Utils::findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties, u32 typeFilter,
    VkMemoryPropertyFlags requiredProperties)
{
    for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        const bool isTypeSuitable        = (typeFilter & (1 << i)) != 0;
        const bool hasRequiredProperties = (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties;

        if (isTypeSuitable && hasRequiredProperties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void Vu::Utils::giveDebugName(const VkDevice device, const VkObjectType objType, const void* objHandle, const char* debugName)
{
#ifndef NDEBUG
    VkDebugUtilsObjectNameInfoEXT info{
        . sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        . pNext = nullptr,
        . objectType = objType,
        . objectHandle = reinterpret_cast<uint64_t>(objHandle),
        . pObjectName = debugName,
    };
    vkSetDebugUtilsObjectNameEXT(device, &info);
#endif
}

VkBool32 Vu::Utils::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    std::cout << "###############################################################################################################\n"
        << "[VALIDATION]: " << pCallbackData->pMessage << "\n"
        << "###############################################################################################################\n"
        << std::endl;
    return VK_FALSE;
}

void Vu::Utils::fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo                 = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

bool Vu::Utils::checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> requestedExtensions)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requested(requestedExtensions.begin(), requestedExtensions.end());

    for (const VkExtensionProperties extension : availableExtensions)
    {
        requested.erase(extension.extensionName);
    }

    if (requested.empty())
    {
        return true;
    }

    for (std::string ext : requested)
    {
        std::cout << "extension not supoorted: " << ext << std::endl;
    }
    return false;
}

bool Vu::Utils::isDeviceSupportExtensions(VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char*> enabledExtensions)
{
    QueueFamilyIndices indices             = QueueFamilyIndices::findQueueFamilies(device, surface);
    bool               extensionsSupported = checkDeviceExtensionSupport(device, enabledExtensions);
    bool               swapChainAdequate   = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(device, surface);
        swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Vu::Utils::checkValidationLayerSupport(std::span<const char*> validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}
