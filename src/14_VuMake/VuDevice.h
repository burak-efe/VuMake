#pragma once

#include <span>

#include "10_Core/VuCommon.h"
#include "11_Config/VuConfig.h"
#include "12_VuMakeCore/VuBuffer.h"
#include "12_VuMakeCore/VuTypes.h"
#include "12_VuMakeCore/VuPools.h"
#include "12_VuMakeCore/VuImage.h"
#include "12_VuMakeCore/VuSampler.h"

#include "VuMaterialDataPool.h"
#include "VuShader.h"

namespace Vu
{
    struct VuDeviceCreateInfo
    {
        const VkBool32                  enableValidationLayers;
        const VkPhysicalDeviceFeatures2 physicalDeviceFeatures2;
        const VkSurfaceKHR              surface;
        const std::span<const char*>    deviceExtensions;
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

        VkPhysicalDeviceMemoryProperties memProperties;

        //private:
        VuPool2<VuImage, 32>   imagePool;
        VuPool2<VuSampler, 32> samplerPool;
        VuPool2<VuBuffer, 32>  bufferPool;
        VuPool2<VuShader, 32>  shaderPool;

        //holds the address of all other buffers
        VuBuffer             bdaBuffer;
        VuBuffer             stagingBuffer;
        VuHandle2<VuBuffer>  debugBuffer;
        VuHandle2<VuImage>   defaultImageHandle;
        VuHandle2<VuImage>   defaultNormalImageHandle;
        VuHandle2<VuSampler> defaultSamplerHandle;
        VuMaterialDataPool   materialDataPool{};
        VuDisposeStack       disposeStack;

    public:
        //RESOURCES
        VuHandle2<VuImage>   createImage(const VuImageCreateInfo& info);
        VuHandle2<VuImage>   createImageFromAsset(const Path& path, VkFormat format);
        VuHandle2<VuSampler> createSampler(const VuSamplerCreateInfo& info);
        VuHandle2<VuBuffer>  createBuffer(const VuBufferCreateInfo& info);
        VuHandle2<VuShader>  createShader(Path vertexPath, Path fragPath, VkRenderPass renderPass);


        VuImage*   get(const VuHandle2<VuImage> handle);
        VuSampler* get(const VuHandle2<VuSampler> handle);
        VuBuffer*  get(const VuHandle2<VuBuffer> handle);
        VuShader*  get(const VuHandle2<VuShader> handle);

        void destroyHandle(VuHandle2<VuImage> handle);
        void destroyHandle(VuHandle2<VuSampler> handle);
        void destroyHandle(VuHandle2<VuBuffer> handle);
        void destroyHandle(VuHandle2<VuShader> handle);

        //INIT
        void uninit();

        void initInstance(VkBool32 enableValidation, std::span<const char*> validationLayers, std::span<const char*> instanceExtensions);

        void initDevice(const VuDeviceCreateInfo& info);

        void initVMA();

        void initCommandPool();

        void initBindlessDescriptor(const VuBindlessConfigInfo& info, uint32 maxFramesInFlight);

        void initDescriptorSetLayout(const VuBindlessConfigInfo& info);

        void initDescriptorPool(const VuBindlessConfigInfo& info);

        void initGlobalDescriptorSet(uint32 maxFramesInFlight);

        void initBindlessManager(const VuBindlessConfigInfo& info);

        void initDefaultResources();

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);


        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void uploadToImage(const VuImage& vuImage, const byte* data, const VkDeviceSize size);

        void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);


        //Decriptor Management
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        //Register the buffer that holds BDA's of all other buffers
        void registerBindlessBDA_Buffer(const VuBuffer& buffer);

        void writeUBO_ToGlobalPool(const VuBuffer& buffer, uint32 writeIndex, uint32 setIndex);

        void registerToBindless(const VuBuffer& buffer, uint32 bindlessIndex);


        void registerToBindless(const VkImageView& imageView, uint32 bindlessIndex);

        void registerToBindless(const VkSampler& sampler, uint32 bindlessIndex);


        //STATIC FUNCTIONS
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
        //                                                     VkDebugUtilsMessageTypeFlagsEXT             messageType,
        //                                                     const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        //                                                     void*                                       pUserData);
        //
        // static void fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        //
        // static VkResult createDebugUtilsMessengerEXT(VkInstance                                instance,
        //                                              const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        //                                              const VkAllocationCallbacks*              pAllocator,
        //                                              VkDebugUtilsMessengerEXT*                 pDebugMessenger);
        //
        // static void destroyDebugUtilsMessengerEXT(VkInstance                   instance,
        //                                           VkDebugUtilsMessengerEXT     debugMessenger,
        //                                           const VkAllocationCallbacks* pAllocator);
        //
        // static void createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger);
        //
        // static bool checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> requestedExtensions);
        //
        // static bool isDeviceSupportExtensions(VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char*> enabledExtensions);
        //
        // static bool checkValidationLayerSupport(std::span<const char*> validationLayers);

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //     void createInstance(bool                   enableValidationLayers,
        //                         std::span<const char*> validationLayers,
        //                         std::span<const char*> extensions,
        //                         VkInstance&            outInstance);
        //
        //     static void createPhysicalDevice(const VkInstance&      instance,
        //                                      const VkSurfaceKHR&    surface,
        //                                      std::span<const char*> enabledExtensions,
        //                                      VkPhysicalDevice&      outPhysicalDevice);
        //
        //
        //     static void createDevice(const VkPhysicalDeviceFeatures2& features,
        //                              const QueueFamilyIndices&        indices,
        //                              const VkPhysicalDevice&          physicalDevice,
        //                              std::span<const char*>           enabledExtensions,
        //                              VkDevice&                        outDevice,
        //                              VkQueue&                         outGraphicsQueue,
        //                              VkQueue&                         outPresentQueue);
        // };
    };
}
