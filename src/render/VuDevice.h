#pragma once

#include <span>
#include <vk_mem_alloc.h>
#include "Common.h"
#include "VuConfig.h"

#include "VuTypes.h"

#include "VuPools.h"
#include "VuImage.h"
#include "VuBuffer.h"
#include "VuSampler.h"
#include "VuShader.h"

namespace Vu
{
    struct VuDeviceCreateInfo
    {
        const VkBool32                   enableValidationLayers;
        const VkPhysicalDeviceFeatures2 physicalDeviceFeatures2;
        const VkSurfaceKHR               surface;
        const std::span<const char*>     deviceExtensions;
    };

    struct VuDevice
    {
        VkInstance                   instance;
        VkDebugUtilsMessengerEXT     debugMessenger;
        VkPhysicalDevice             physicalDevice;
        QueueFamilyIndices           queueFamilyIndices;
        VkDevice                     device;
        VkQueue                      graphicsQueue;
        VkQueue                      presentQueue;
        VkCommandPool                commandPool;
        VkDescriptorSetLayout        globalDescriptorSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        VkDescriptorPool             descriptorPool;
        VkDescriptorPool             uiDescriptorPool;
        VkPipelineLayout             globalPipelineLayout;
        VmaAllocator                 vma;

        VuPool2<VuImage, 32>   imagePool;
        VuPool2<VuSampler, 32> samplerPool;
        VuPool2<VuBuffer, 32>  bufferPool;
        VuPool2<VuShader, 32>  shaderPool;

        VuImage*   get(const VuHandle2<VuImage> handle) { return imagePool.get(handle); }
        VuSampler* get(const VuHandle2<VuSampler> handle) { return samplerPool.get(handle); }
        VuBuffer*  get(const VuHandle2<VuBuffer> handle) { return bufferPool.get(handle); }
        VuShader*  get(const VuHandle2<VuShader> handle) { return shaderPool.get(handle); }


        VuDisposeStack disposeStack;

        void uninit();

        void initInstance(VkBool32               enableValidationLayers, std::span<const char*> validationLayers,
                          std::span<const char*> instanceExtensions);

        void initDevice(const VuDeviceCreateInfo& info);

        void initVMA();

        void initCommandPool();

        void initBindless(const VuBindlessConfigInfo& info, uint32 maxFramesInFligth);

        void initDescriptorSetLayout(const VuBindlessConfigInfo& info);

        void initDescriptorPool(const VuBindlessConfigInfo& info);

        void initGlobalDescriptorSet(uint32 maxFramesInFlight);

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        //STATIC FUNCTIONS///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                                                            VkDebugUtilsMessageTypeFlagsEXT             messageType,
                                                            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                            void*                                       pUserData);

        static void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

        static VkResult createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks*              pAllocator,
                                                     VkDebugUtilsMessengerEXT*                 pDebugMessenger);

        static void destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                  VkDebugUtilsMessengerEXT     debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator);

        static void createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger);

        static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> requestedExtensions);

        static bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char*> enabledExtensions);

        static bool checkValidationLayerSupport(std::span<const char*> validationLayers);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void createInstance(bool                   enableValidationLayers,
                            std::span<const char*> validationLayers,
                            std::span<const char*> extensions,
                            VkInstance&            outInstance);

        static void createPhysicalDevice(const VkInstance&      instance,
                                         const VkSurfaceKHR&    surface,
                                         std::span<const char*> enabledExtensions,
                                         VkPhysicalDevice&      outPhysicalDevice);


        static void createDevice(const VkPhysicalDeviceFeatures2& features,
                                 const QueueFamilyIndices&        indices,
                                 const VkPhysicalDevice&          physicalDevice,
                                 std::span<const char*>           enabledExtensions,
                                 VkDevice&                        outDevice,
                                 VkQueue&                         outGraphicsQueue,
                                 VkQueue&                         outPresentQueue);
    };
}
