#pragma once
#include <set>

#include "VuSwapChain.h"
#include "VuTypes.h"
#include "vulkan/vulkan.h"

namespace VuDeviceUtils {
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

            if (indices.IsComplete()) {
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
            SwapChainSupportDetails swapChainSupport = Vu::VuSwapChain::QuerySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }


        return queue_family_indices.IsComplete() && extensionsSupported && swapChainAdequate;
    }


    inline void PickPhysicalDevice(VkSurfaceKHR surface) {
        uint32 deviceCount = 0;

        vkEnumeratePhysicalDevices(VuContext::Instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);

        vkEnumeratePhysicalDevices(VuContext::Instance, &deviceCount, devices.data());

        for (const auto &device: devices) {
            if (IsDeviceSuitable(device, surface)) {
                VuContext::PhysicalDevice = device;
                break;
            }
        }

        if (VuContext::PhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
        VkPhysicalDeviceProperties deviceProperties = {};
        vkGetPhysicalDeviceProperties(VuContext::PhysicalDevice, &deviceProperties);
        std::cout << "Selected device: " << deviceProperties.deviceName << "\n";
    }


    inline void CreateLogicalDevice(VkSurfaceKHR& surface, VkQueue& graphicsQueue, VkQueue& presentQueue) {
        QueueFamilyIndices indices = VuDeviceUtils::findQueueFamilies(VuContext::PhysicalDevice, surface);

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


        constexpr VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamic_rendering_feature {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .dynamicRendering = VK_TRUE,
        };

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &dynamic_rendering_feature;

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

        VK_CHECK(vkCreateDevice(VuContext::PhysicalDevice, &createInfo, nullptr, &VuContext::Device))

        vkGetDeviceQueue(VuContext::Device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(VuContext::Device, indices.presentFamily.value(), 0, &presentQueue);
    }
};
