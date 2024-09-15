#pragma once

#include <cassert>
#include <vulkan/vulkan_core.h>
#include "Common.h"
#include "VuUtils.h"
#include "GLFW/glfw3.h"
#include <print>

namespace VuInit {
    // static VkResult CreateDebugUtilsMessengerEXT(
    //     VkInstance instance,
    //     const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
    //     const VkAllocationCallbacks *pAllocator,
    //     VkDebugUtilsMessengerEXT *pDebugMessenger) {
    //     auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
    //         instance, "vkCreateDebugUtilsMessengerEXT"));
    //
    //     if (func != nullptr) {
    //         return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    //     }
    //     return VK_ERROR_EXTENSION_NOT_PRESENT;
    // }
    //
    static void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }


    // static bool CheckValidationLayerSupport(const std::vector<const char *> &validationLayers) {
    //     uint32 layerCount;
    //     vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    //
    //     std::vector<VkLayerProperties> availableLayers(layerCount);
    //     vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    //
    //
    //     for (const char *layerName: validationLayers) {
    //         bool layerFound = false;
    //         for (const auto &layerProperties: availableLayers) {
    //             if (strcmp(layerName, layerProperties.layerName) == 0) {
    //                 layerFound = true;
    //                 break;
    //             }
    //         }
    //
    //         if (!layerFound) {
    //             return false;
    //         }
    //     }
    //     return true;
    // }

    // static VkBool32 DebugCallback(
    //     VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    //     VkDebugUtilsMessageSeverityFlagsEXT messageType,
    //     const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    //     void *pUserData) {
    //     std::cerr << std::endl << pCallbackData->pMessage << std::endl;
    //     return VK_FALSE;
    // }
    //
    // static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
    //     createInfo = {};
    //     createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    //     createInfo.messageSeverity =
    //             //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
    //             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
    //             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //     createInfo.messageType =
    //             VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
    //             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
    //             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    //     createInfo.pfnUserCallback = DebugCallback;
    //     createInfo.pUserData = nullptr; // Optional
    // }
    //
    // static std::vector<const char *> GetRequiredExtensions(bool enableValidationLayers) {
    //     uint32 glfwExtensionCount = 0;
    //     const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    //
    //     std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    //
    //     if (enableValidationLayers) {
    //         extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    //     }
    //
    //     return extensions;
    // }


    // static VkInstance CreateVulkanInstance(
    //     bool enableValidationLayers,
    //     const std::vector<const char *> &validationLayers) {
    //     if (enableValidationLayers && !CheckValidationLayerSupport(validationLayers)) {
    //         throw std::runtime_error("validation layers requested, but not available!");
    //     }
    //
    //     VkApplicationInfo appInfo{};
    //     appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    //     appInfo.pApplicationName = "Hello Triangle";
    //     appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    //     appInfo.pEngineName = "None :C";
    //     appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    //     appInfo.apiVersion = VK_API_VERSION_1_3;
    //
    //     VkInstanceCreateInfo createInfo{};
    //     createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    //     createInfo.pApplicationInfo = &appInfo;
    //
    //     //Get Extensions
    //     auto extension = GetRequiredExtensions(enableValidationLayers);
    //     createInfo.enabledExtensionCount = static_cast<uint32>(extension.size());
    //     createInfo.ppEnabledExtensionNames = extension.data();
    //
    //
    //     //enable validation layers if in debug mode
    //     VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    //     if (enableValidationLayers) {
    //         createInfo.enabledLayerCount = static_cast<uint32>(validationLayers.size());
    //         createInfo.ppEnabledLayerNames = validationLayers.data();
    //         PopulateDebugMessengerCreateInfo(debugCreateInfo);
    //         createInfo.pNext = &debugCreateInfo;
    //     } else {
    //         createInfo.enabledLayerCount = 0;
    //         createInfo.pNext = nullptr;
    //     }
    //
    //
    //     VkInstance instance;
    //     // Try create vkInstance
    //     VK_CHECK(vkCreateInstance(&createInfo, nullptr, &instance));
    //
    //     uint32 extensionCount = 0;
    //     vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    //     std::vector<VkExtensionProperties> extensions(extensionCount);
    //     vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    //
    //     // std::cout << "available extensions:\n";
    //     // for (const auto &vk_extension_property: extensions) {
    //     //     std::cout << "\t" << vk_extension_property.extensionName << "\n";
    //     // }
    //
    //     return instance;
    // }


    // static VkDebugUtilsMessengerEXT CreateDebugMessenger(VkInstance instance) {
    //     //if constexpr (!enableValidationLayers) return;
    //
    //     VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    //     PopulateDebugMessengerCreateInfo(createInfo);
    //
    //     VkDebugUtilsMessengerEXT debugMessenger;
    //     VK_CHECK(VuInit::CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger))
    //     return debugMessenger;
    // }
    //

    inline VkImageMemoryBarrier ImageMemoryBarrier() {
        VkImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageMemoryBarrier;
    }

    inline void InsertImageMemoryBarrier(
        VkCommandBuffer cmdbuffer,
        VkImage image,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkImageLayout oldImageLayout,
        VkImageLayout newImageLayout,
        VkPipelineStageFlags srcStageMask,
        VkPipelineStageFlags dstStageMask,
        VkImageSubresourceRange subresourceRange) {
        VkImageMemoryBarrier imageMemoryBarrier = ImageMemoryBarrier();
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0, nullptr,
            0, nullptr,
            1, &imageMemoryBarrier
        );
    }
}
