#pragma once

#include <iostream>
#include <set>
#include <span>

#include <vk_mem_alloc.h>

#include "Common.h"
#include "VuConfig.h"
#include "VuTypes.h"
#include "VuUtils.h"

namespace Vu {

    struct VuDeviceCreateInfo {
        const VkBool32                   enableValidationLayers;
        const VkPhysicalDeviceFeatures2& physicalDeviceFeatures2;
        const VkSurfaceKHR               surface;
        const std::span<const char *>    deviceExtensions;
    };

    struct VuDevice {
        VkInstance                   instance;
        VkDebugUtilsMessengerEXT     debugMessenger;
        VkPhysicalDevice             physicalDevice;
        QueueFamilyIndices           queueFamilyIndices;
        VkDevice                     device;
        VkQueue                      graphicsQueue;
        VkQueue                      presentQueue;
        VmaAllocator                 vma;
        VkCommandPool                commandPool;
        VkDescriptorSetLayout        globalDescriptorSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        VkDescriptorPool             descriptorPool;
        VkDescriptorPool             uiDescriptorPool;
        VkPipelineLayout             globalPipelineLayout;

        VuDisposeStack disposeStack;

        void uninit() {
            disposeStack.disposeAll();
        }

        void initInstance(VkBool32                enableValidationLayers,
                          std::span<const char *> validationLayers,
                          std::span<const char *> instanceExtensions) {

            createInstance(enableValidationLayers, validationLayers, instanceExtensions, instance);
            disposeStack.push([&] {
                vkDestroyInstance(instance, nullptr);
            });

            if (enableValidationLayers) {
                createDebugMessenger(instance, debugMessenger);
                disposeStack.push([&] {
                    destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
                });
            }
        }

        void initDevice(const VuDeviceCreateInfo& info) {
            createPhysicalDevice(instance, info.surface, info.deviceExtensions, physicalDevice);
            queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(physicalDevice, info.surface);

            createDevice(info.physicalDeviceFeatures2,
                         queueFamilyIndices,
                         physicalDevice,
                         info.deviceExtensions,
                         device, graphicsQueue, presentQueue
                        );
            disposeStack.push([&] {
                vkDestroyDevice(device, nullptr);
            });

            initVMA();
            disposeStack.push([&] {
                vmaDestroyAllocator(vma);
            });
            initCommandPool();
            disposeStack.push([&] {
                vkDestroyCommandPool(device, commandPool, nullptr);
            });
        }

        void initVMA() {
            ZoneScoped;
            VmaVulkanFunctions vma_vulkan_func{
                .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
                .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
                .vkAllocateMemory = vkAllocateMemory,
                .vkFreeMemory = vkFreeMemory,
                .vkMapMemory = vkMapMemory,
                .vkUnmapMemory = vkUnmapMemory,
                .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
                .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
                .vkBindBufferMemory = vkBindBufferMemory,
                .vkBindImageMemory = vkBindImageMemory,
                .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
                .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
                .vkCreateBuffer = vkCreateBuffer,
                .vkDestroyBuffer = vkDestroyBuffer,
                .vkCreateImage = vkCreateImage,
                .vkDestroyImage = vkDestroyImage,
                .vkCmdCopyBuffer = vkCmdCopyBuffer,
            };

            VmaAllocatorCreateInfo createInfo{
                .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
                .physicalDevice = physicalDevice,
                .device = device,
                .pVulkanFunctions = &vma_vulkan_func,
                .instance = instance,
            };

            VkCheck(vmaCreateAllocator(&createInfo, &vma));
        }

        void initCommandPool() {
            ZoneScoped;
            VkCommandPoolCreateInfo poolInfo{};
            poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
            VkCheck(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
        }

        void initBindless(const VuBindlessConfigInfo& info, uint32 maxFramesInFligth) {
            initDescriptorSetLayout(info);
            disposeStack.push([&] {
                vkDestroyDescriptorSetLayout(device, globalDescriptorSetLayout, nullptr);

            });
            initDescriptorPool(info);
            disposeStack.push([&] {
                vkDestroyDescriptorPool(device, descriptorPool, nullptr);

            });
            initGlobalDescriptorSet(maxFramesInFligth);
            std::array descSetLayouts{globalDescriptorSetLayout};

            createPipelineLayout(device, descSetLayouts, config::PUSH_CONST_SIZE, globalPipelineLayout);
            disposeStack.push([&] {
                vkDestroyPipelineLayout(device, globalPipelineLayout, nullptr);
            });
        }

        void initDescriptorSetLayout(const VuBindlessConfigInfo& info) {
            ZoneScoped;
            VkDescriptorSetLayoutBinding ubo{
                .binding = info.uboBinding,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = info.uboCount,
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            VkDescriptorSetLayoutBinding sampler{
                .binding = info.samplerBinding,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
                .descriptorCount = info.samplerCount,
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            VkDescriptorSetLayoutBinding sampledImage{
                .binding = info.sampledImageBinding,
                .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                .descriptorCount = info.sampledImageCount,
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            VkDescriptorSetLayoutBinding storageImage{
                .binding = info.storageImageBinding,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                .descriptorCount = info.storageImageCount,
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            VkDescriptorSetLayoutBinding uniformBuffer{
                .binding = info.uniformBufferBinding,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            VkDescriptorSetLayoutBinding storageBuffer{
                .binding = info.storageBufferBinding,
                .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                .descriptorCount = 1,
                .stageFlags = VK_SHADER_STAGE_ALL,
            };
            std::array descriptorSetLayoutBindings{
                ubo,
                sampler,
                sampledImage,
                storageImage,
                uniformBuffer,
                storageBuffer,
            };
            VkDescriptorSetLayoutCreateInfo globalSetLayout{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
                .bindingCount = descriptorSetLayoutBindings.size(),
                .pBindings = descriptorSetLayoutBindings.data(),
            };

            const VkDescriptorBindingFlagsEXT flag =
                    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
                    | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
                    | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

            std::array descriptorSetLayoutFlags{flag, flag, flag, flag, flag, flag};

            VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
            binding_flags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
            binding_flags.bindingCount  = descriptorSetLayoutFlags.size();
            binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();
            globalSetLayout.pNext       = &binding_flags;

            VkCheck(vkCreateDescriptorSetLayout(device, &globalSetLayout, nullptr, &globalDescriptorSetLayout));
        }

        void initDescriptorPool(const VuBindlessConfigInfo& info) {
            ZoneScoped;

            std::array<VkDescriptorPoolSize, 5> poolSizes{
                {
                    {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = info.uboCount},
                    {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = info.samplerCount},
                    {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = info.sampledImageCount},
                    {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = info.storageImageCount},
                    {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = info.storageBufferCount},
                },
            };

            VkDescriptorPoolCreateInfo poolInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
                .maxSets = 2,
                .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
                .pPoolSizes = poolSizes.data(),
            };
            VkCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
        }

        void initGlobalDescriptorSet(uint32 maxFramesInFligth) {
            ZoneScoped;
            std::vector globalLayouts(maxFramesInFligth, globalDescriptorSetLayout);

            VkDescriptorSetAllocateInfo globalSetsAllocInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = descriptorPool,
                .descriptorSetCount = maxFramesInFligth,
                .pSetLayouts = globalLayouts.data(),
            };

            globalDescriptorSets.resize(maxFramesInFligth);
            VkCheck(vkAllocateDescriptorSets(device, &globalSetsAllocInfo, globalDescriptorSets.data()));
        }


        VkCommandBuffer BeginSingleTimeCommands() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool        = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            VkCheck(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

            vkBeginCommandBuffer(commandBuffer, &beginInfo);

            return commandBuffer;
        }

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
            vkEndCommandBuffer(commandBuffer);

            VkSubmitInfo submitInfo{};
            submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.commandBufferCount = 1;
            submitInfo.pCommandBuffers    = &commandBuffer;

            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphicsQueue);

            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

        //STATIC FUNCTIONS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void*                                       pUserData) {
            std::cout << "###############################################################################################################\n"
                    << "[VALIDATION]: " << pCallbackData->pMessage << "\n"
                    << "###############################################################################################################\n";
            return VK_FALSE;
        }

        static void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
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

        static VkResult createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks*              pAllocator,
                                                     VkDebugUtilsMessengerEXT*                 pDebugMessenger) {
            auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
                 instance, "vkCreateDebugUtilsMessengerEXT"));
            if (func != nullptr) {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
            } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
        }

        static void destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                  VkDebugUtilsMessengerEXT     debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator) {
            auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
                 instance, "vkDestroyDebugUtilsMessengerEXT"));
            if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
            }
        }


        static void createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger) {
            VkDebugUtilsMessengerCreateInfoEXT createInfo{};
            fillDebugMessengerCreateInfo(createInfo);
            VkCheck(createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &outDebugMessenger));
        }

        static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char *> requestedExtensions) {
            uint32_t extensionCount = 0;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

            std::set<std::string> requested(requestedExtensions.begin(), requestedExtensions.end());

            for (const VkExtensionProperties extension: availableExtensions) {
                requested.erase(extension.extensionName);
            }

            if (requested.empty()) {
                return true;
            }

            for (std::string ext: requested) {
                std::cout << "extension not supoorted: " << ext << std::endl;
            }
            return false;
        }

        static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char *> enabledExtensions) {
            QueueFamilyIndices indices             = QueueFamilyIndices::findQueueFamilies(device, surface);
            bool               extensionsSupported = checkDeviceExtensionSupport(device, enabledExtensions);
            bool               swapChainAdequate   = false;
            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(device, surface);
                swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

            return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
        }


        static bool checkValidationLayerSupport(std::span<const char *> validationLayers) {
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

            for (const auto layerName: validationLayers) {
                bool layerFound = false;

                for (const auto& layerProperties: availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }

                if (!layerFound) {
                    return false;
                }
            }

            return true;
        }

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void createInstance(bool                    enableValidationLayers,
                            std::span<const char *> validationLayers,
                            std::span<const char *> extensions,
                            VkInstance&             outInstance) {
            ZoneScoped;

            if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) {
                throw std::runtime_error("validation layers requested, but not available!");
            }

            VkApplicationInfo appInfo{};
            appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
            appInfo.pApplicationName   = "VuMake";
            appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
            appInfo.pEngineName        = "No Engine";
            appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
            appInfo.apiVersion         = VK_API_VERSION_1_2;

            VkInstanceCreateInfo instanceCreateInfo{};
            instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            instanceCreateInfo.pApplicationInfo = &appInfo;

            instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
            instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

            VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
            if (enableValidationLayers) {
                instanceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
                instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
                fillDebugMessengerCreateInfo(debugCreateInfo);

                //
                // const char* layer_name = "VK_LAYER_KHRONOS_validation";
                //
                // const VkBool32 setting_validate_core = VK_TRUE;
                // const VkBool32 setting_validate_sync = VK_FALSE;
                // const VkBool32 setting_thread_safety = VK_TRUE;
                // const char* setting_debug_action[] = {"VK_DBG_LAYER_ACTION_LOG_MSG"};
                // const char* setting_report_flags[] = {"info", "warn", "perf", "error"};
                // const VkBool32 setting_enable_message_limit = VK_TRUE;
                // const int32_t setting_duplicate_message_limit = 3;
                //
                // const VkLayerSettingEXT settings[] = {
                //     {layer_name, "validate_core", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_core},
                //     {layer_name, "validate_sync", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_validate_sync},
                //     {layer_name, "thread_safety", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_thread_safety},
                //     {layer_name, "debug_action", VK_LAYER_SETTING_TYPE_STRING_EXT, 1, setting_debug_action},
                //     {
                //         layer_name, "report_flags", VK_LAYER_SETTING_TYPE_STRING_EXT,
                //         static_cast<uint32_t>(std::size(setting_report_flags)), setting_report_flags
                //     },
                //     {layer_name, "enable_message_limit", VK_LAYER_SETTING_TYPE_BOOL32_EXT, 1, &setting_enable_message_limit},
                //     {layer_name, "duplicate_message_limit", VK_LAYER_SETTING_TYPE_INT32_EXT, 1, &setting_duplicate_message_limit}
                // };
                //
                // const VkLayerSettingsCreateInfoEXT layer_settings_create_info = {
                //     VK_STRUCTURE_TYPE_LAYER_SETTINGS_CREATE_INFO_EXT,
                //     &debugCreateInfo,
                //     static_cast<uint32_t>(std::size(settings)),
                //     settings
                // };


                instanceCreateInfo.pNext = &debugCreateInfo;
            }
            //create
            {
                ZoneScopedN("create");
                VkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &outInstance));
            }
        }

        static void createPhysicalDevice(const VkInstance&       instance,
                                         const VkSurfaceKHR&     surface,
                                         std::span<const char *> enabledExtensions,
                                         VkPhysicalDevice&       outPhysicalDevice) {
            ZoneScoped;
            outPhysicalDevice    = VK_NULL_HANDLE;
            uint32_t deviceCount = 0;
            vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

            if (deviceCount == 0) {
                throw std::runtime_error("failed to find GPUs with Vulkan support!");
            }

            std::vector<VkPhysicalDevice> devices(deviceCount);
            vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

            for (const auto& device: devices) {
                if (isDeviceSuitable(device, surface, enabledExtensions)) {
                    outPhysicalDevice = device;
                    break;
                }
            }

            if (outPhysicalDevice == VK_NULL_HANDLE) {
                throw std::runtime_error(std::string("failed to find a suitable GPU!"));
            }
        }


        static void createDevice(const VkPhysicalDeviceFeatures2& features,
                                 const QueueFamilyIndices&        indices,
                                 const VkPhysicalDevice&          physicalDevice,
                                 std::span<const char *>          enabledExtensions,
                                 VkDevice&                        outDevice,
                                 VkQueue&                         outGraphicsQueue,
                                 VkQueue&                         outPresentQueue) {
            ZoneScoped;
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
            std::set<uint32_t>                   uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

            float queuePriority = 1.0f;
            for (uint32_t queueFamily: uniqueQueueFamilies) {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamily;
                queueCreateInfo.queueCount       = 1;
                queueCreateInfo.pQueuePriorities = &queuePriority;
                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkDeviceCreateInfo createInfo{};
            createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.pNext                   = &features;
            createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos       = queueCreateInfos.data();
            createInfo.pEnabledFeatures        = nullptr;
            createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
            createInfo.ppEnabledExtensionNames = enabledExtensions.data();
            //create
            {
                ZoneScopedN("create");
                VkCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &outDevice));
            }

            //queue
            {
                ZoneScopedN("Queue");
                vkGetDeviceQueue(outDevice, indices.graphicsFamily.value(), 0, &outGraphicsQueue);
                vkGetDeviceQueue(outDevice, indices.presentFamily.value(), 0, &outPresentQueue);
            }
        }
    };
}
