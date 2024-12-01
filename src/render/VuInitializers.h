#pragma once
#include <set>

#include"Common.h"
#include "VuConfig.h"

namespace Vu::Initializers {

    inline std::vector<const char *> getRequiredExtensions() {
        // uint32_t glfwExtensionCount = 0;
        // const char** glfwExtensions;
        Uint32 count_instance_extensions;
        const char* const * instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count_instance_extensions);

        std::vector<const char *> extensions(instance_extensions, instance_extensions + count_instance_extensions);

        if (config::ENABLE_VALIDATION_LAYERS_LAYERS) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    inline void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }


    inline VkResult createDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    inline void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                              const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }


    inline void createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger) {
        if (!config::ENABLE_VALIDATION_LAYERS_LAYERS) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &outDebugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    inline bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(config::DEVICE_EXTENSIONS.begin(), config::DEVICE_EXTENSIONS.end());

        for (const auto& extension: availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    inline bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
        QueueFamilyIndices indices = QueueFamilyIndices::findQueueFamilies(device, surface);

        bool extensionsSupported = checkDeviceExtensionSupport(device);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = VuSwapChain::querySwapChainSupport(device, surface);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }

        VkPhysicalDeviceFeatures supportedFeatures;
        vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

        return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    inline void createInstance(VkInstance& outInstance) {
        ZoneScoped;

        // if (enableValidationLayers && !checkValidationLayerSupport()) {
        //     throw std::runtime_error("validation layers requested, but not available!");
        // }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (config::ENABLE_VALIDATION_LAYERS_LAYERS) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(config::VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = config::VALIDATION_LAYERS.data();

            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        } {
            ZoneScopedN("create");
            if (vkCreateInstance(&createInfo, nullptr, &outInstance) != VK_SUCCESS) {
                throw std::runtime_error("failed to create instance!");
            }
        }
    }

    inline void createPhysicalDevice(const VkInstance& instance,
                                     const VkSurfaceKHR& surface,
                                     VkPhysicalDevice& outPhysicalDevice) {
        ZoneScoped;

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto& device: devices) {
            if (isDeviceSuitable(device, surface)) {
                outPhysicalDevice = device;
                break;
            }
        }

        if (outPhysicalDevice == VK_NULL_HANDLE) {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }


    inline void createDevice(const VkPhysicalDeviceFeatures2& features, const QueueFamilyIndices& indices,
                             const VkPhysicalDevice& physicalDevice,
                             VkDevice& outDevice,
                             VkQueue& outGraphicsQueue,
                             VkQueue& outPresentQueue) {
        ZoneScoped;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily: uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pNext = &features;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = nullptr;

        createInfo.enabledExtensionCount = static_cast<uint32_t>(config::DEVICE_EXTENSIONS.size());
        createInfo.ppEnabledExtensionNames = config::DEVICE_EXTENSIONS.data();

        if (config::ENABLE_VALIDATION_LAYERS_LAYERS) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(config::VALIDATION_LAYERS.size());
            createInfo.ppEnabledLayerNames = config::VALIDATION_LAYERS.data();
        } else {
            createInfo.enabledLayerCount = 0;
        } {
            ZoneScopedN("create");
            if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &outDevice) != VK_SUCCESS) {
                throw std::runtime_error("failed to create logical device!");
            }
        }

        //queue
        {
            ZoneScopedN("Queue");
            vkGetDeviceQueue(outDevice, indices.graphicsFamily.value(), 0, &outGraphicsQueue);
            vkGetDeviceQueue(outDevice, indices.presentFamily.value(), 0, &outPresentQueue);
        }

    }


}
