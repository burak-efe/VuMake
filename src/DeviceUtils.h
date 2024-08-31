#pragma once
#include <set>

#include "SwapChain.h"
#include "VulkanTypes.h"
#include "vulkan/vulkan.h"

namespace DeviceUtils {
    inline bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32 extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());


        // for (auto extension: availableExtensions) {
        //     std::cout << (extension.extensionName) << "\n";
        // }

        std::set<std::string> requiredExtensions(k_DeviceExtensions.begin(), k_DeviceExtensions.end());

        for (const auto &extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    inline QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
        //Logic to find graphics queue family
        QueueFamilyIndices indices;

        uint32 queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;


        for (const auto &queuefamily: queueFamilies) {
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


    inline bool IsDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices queue_family_indices = findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = VuMake::SwapChain::querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }


        return queue_family_indices.isComplete() && extensionsSupported && swapChainAdequate;
    }


    inline void PickPhysicalDevice(VkSurfaceKHR surface) {
        uint32 deviceCount = 0;

        vkEnumeratePhysicalDevices(EngineContext::Instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);

        vkEnumeratePhysicalDevices(EngineContext::Instance, &deviceCount, devices.data());

        for (const auto &device: devices) {
            if (IsDeviceSuitable(device, surface)) {
                EngineContext::PhysicalDevice = device;
                break;
            }
        }

        if (EngineContext::PhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(EngineContext::PhysicalDevice, &deviceProperties);
        std::cout << "Selected device: " << deviceProperties.deviceName << "\n";
    }


    inline void CreateLogicalDevice(VkSurfaceKHR& surface, VkQueue& graphicsQueue, VkQueue& presentQueue) {
        QueueFamilyIndices indices = DeviceUtils::findQueueFamilies(EngineContext::PhysicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;

        for (uint32 queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32>(k_DeviceExtensions.size());
        createInfo.ppEnabledExtensionNames = k_DeviceExtensions.data();

        if (enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32>(k_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = k_ValidationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        VK_CHECK(vkCreateDevice(EngineContext::PhysicalDevice, &createInfo, nullptr, &EngineContext::Device))

        vkGetDeviceQueue(EngineContext::Device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(EngineContext::Device, indices.presentFamily.value(), 0, &presentQueue);
    }
};
