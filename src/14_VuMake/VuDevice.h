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
#include "12_VuMakeCore/VuGraphicsPipeline.h"

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
        VuResourcePool<VuImage, 32>    imagePool;
        VuResourcePool<VuSampler, 32>  samplerPool;
        VuResourcePool<VuBuffer, 32>   bufferPool;
        VuResourcePool<VuShader, 32>   shaderPool;
        VuResourcePool<VuMaterial, 32> materialPool;
        VuResourcePool<uint32, 32>     materialDataIndexPool;

        //holds the address of all other buffers
        VuBuffer             bdaBuffer;
        VuBuffer             stagingBuffer;
        VuHnd<VuBuffer>  debugBuffer;
        VuHnd<VuBuffer>  materialDataBufferHandle;
        VuHnd<VuImage>   defaultImageHandle;
        VuHnd<VuImage>   defaultNormalImageHandle;
        VuHnd<VuSampler> defaultSamplerHandle;
        //VuMaterialDataPool   materialDataPool{};
        VuDisposeStack disposeStack;

    public:
        //RESOURCES
        VuImage*    getImage(const VuHnd<VuImage> handle);
        VuSampler*  getSampler(const VuHnd<VuSampler> handle);
        VuBuffer*   getBuffer(const VuHnd<VuBuffer> handle);
        VuShader*   getShader(const VuHnd<VuShader> handle);
        VuMaterial* getMaterial(const VuHnd<VuMaterial> handle);
        uint32*     getMaterialDataIndex(const VuHnd<uint32> handle);

        void destroyHandle(VuHnd<VuImage> handle);
        void destroyHandle(VuHnd<VuSampler> handle);
        void destroyHandle(VuHnd<VuBuffer> handle);
        void destroyHandle(VuHnd<VuShader> handle);
        void destroyHandle(VuHnd<VuMaterial> handle);
        void destroyHandle(VuHnd<uint32> handle);

        VuHnd<VuImage>   createImage(const VuImageCreateInfo& info);
        VuHnd<VuImage>   createImageFromAsset(const Path& path, VkFormat format);
        VuHnd<VuSampler> createSampler(const VuSamplerCreateInfo& info);
        VuHnd<VuBuffer>  createBuffer(const VuBufferCreateInfo& info);
        VuHnd<VuShader>  createShader(Path vertexPath, Path fragPath, VkRenderPass renderPass);

        VuHnd<uint32> createMaterialDataIndex()
        {
            VuHnd<uint32> handle   = materialDataIndexPool.createHandle();
            uint32*           resource = getMaterialDataIndex(handle);
            assert(resource != nullptr);
            *resource = handle.index;
            return handle;
        }

        VuHnd<VuMaterial> createMaterial(MaterialSettings matSettings, VuHnd<VuShader> shaderHnd, VuHnd<uint32> materialDataHnd)
        {
            VuHnd<VuMaterial> handle   = materialPool.createHandle();
            VuMaterial*           resource = getMaterial(handle);
            assert(resource != nullptr);
            resource->init(this, matSettings, shaderHnd, materialDataHnd);
            return handle;
        }

        GPU_PBR_MaterialData* getMaterialData(VuHnd<uint32> handle)
        {
            VuBuffer* matDataBuffer = getBuffer(materialDataBufferHandle);
            byte*     dataPtr       = static_cast<byte*>(matDataBuffer->mapPtr) + config::MATERIAL_DATA_SIZE * handle.index;
            return reinterpret_cast<GPU_PBR_MaterialData*>(dataPtr);
        }

        void bindMaterial(VkCommandBuffer cb, VuHnd<VuMaterial> material)
        {
            auto*              mat      = getMaterial(material);
            auto*              shader   = getShader(mat->shaderHnd);
            VuGraphicsPipeline pipeline = shader->requestPipeline(mat->materialSettings);

            vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
        }


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
