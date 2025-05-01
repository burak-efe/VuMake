#pragma once

#include <span>

#include "10_Core/VuCommon.h"
#include "12_VuMakeCore/VuBuffer.h"
#include "12_VuMakeCore/VuTypes.h"
#include "../08_LangUtils/VuPools.h"
#include "12_VuMakeCore/VuImage.h"
#include "12_VuMakeCore/VuSampler.h"

#include "VuShader.h"

namespace Vu
{
    struct VuDeviceCreateInfo
    {
        VkInstance                instance;
        VkPhysicalDevice          physicalDevice;
        VkBool32                  enableValidationLayers;
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures2;
        VkSurfaceKHR              surface;
        std::span<const char*>    deviceExtensions;

        //bindless
        u32 uboBinding;
        u32 samplerBinding;
        u32 sampledImageBinding;
        u32 storageImageBinding;
        u32 storageBufferBinding;
        u32 uboCount;
        u32 samplerCount;
        u32 sampledImageCount;
        u32 storageImageCount;
        u32 storageBufferCount;
        //pool handles
        PoolHandle imagePoolHnd;
        PoolHandle samplerPoolHnd;
        PoolHandle bufferPoolHnd;
        PoolHandle shaderPoolHnd;
        PoolHandle materialPoolHnd;
        PoolHandle materialDataIndexPoolHnd;
    };

    struct VuDevice
    {
        VkInstance       instance;
        VkPhysicalDevice physicalDevice;

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

        PoolHandle imagePoolHnd;
        PoolHandle samplerPoolHnd;
        PoolHandle bufferPoolHnd;
        PoolHandle shaderPoolHnd;
        PoolHandle materialPoolHnd;
        PoolHandle materialDataIndexPoolHnd;

    private:
        VuBuffer         bdaBuffer; //holds the address of all other buffers
        VuBuffer         stagingBuffer;
        VuHnd<VuBuffer>  debugBufferHnd;
        VuHnd<VuBuffer>  materialDataBufferHandle;
        VuHnd<VuImage>   defaultImageHandle;
        VuHnd<VuImage>   defaultNormalImageHandle;
        VuHnd<VuSampler> defaultSamplerHandle;
        VuDisposeStack   disposeStack;

    public:
        //RESOURCES
        // VuImage*    getImage(const VuHnd<VuImage> handle);
        // VuSampler*  getSampler(const VuHnd<VuSampler> handle);
        // VuBuffer*   getBuffer(const VuHnd<VuBuffer> handle);
        // VuShader*   getShader(const VuHnd<VuShader> handle);
        // VuMaterial* getMaterial(const VuHnd<VuMaterial> handle);
        // u32*        getMaterialDataIndex(const VuHnd<u32> handle);
        //
        // void destroyHandle(VuHnd<VuImage> handle);
        // void destroyHandle(VuHnd<VuSampler> handle);
        // void destroyHandle(VuHnd<VuBuffer> handle);
        // void destroyHandle(VuHnd<VuShader> handle);
        // void destroyHandle(VuHnd<VuMaterial> handle);
        // void destroyHandle(VuHnd<u32> handle);

        VuHnd<VuImage>   createImage(const VuImageCreateInfo& info);
        VuHnd<VuImage>   createImageFromAsset(const path& path, VkFormat format);
        VuHnd<VuSampler> createSampler(const VuSamplerCreateInfo& info);
        VuHnd<VuBuffer>  createBuffer(const VuBufferCreateInfo& info);
        VuHnd<VuShader>  createShader(path vertexPath, path fragPath, VuRenderPass* vuRenderPass);

        VuHnd<u32> createMaterialDataIndex();

        VuHnd<VuMaterial> createMaterial(MaterialSettings matSettings, VuHnd<VuShader> shaderHnd, VuHnd<u32> materialDataHnd);

        std::span<byte, 64> getMaterialData(VuHnd<u32> handle);

        void bindMaterial(VkCommandBuffer cb, VuHnd<VuMaterial> material);

        //INIT

        void init(const VuDeviceCreateInfo& info);

        void uninit();

        void registerBindlessBDA_Buffer(const VuBuffer& buffer);

        void writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex);

        void registerToBindless(const VuBuffer& buffer, u32 bindlessIndex);

        void registerToBindless(const VkImageView& imageView, u32 bindlessIndex);

        void registerToBindless(const VkSampler& sampler, u32 bindlessIndex);

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

        void uploadToImage(const VuImage& vuImage, const byte* data, const VkDeviceSize size);

        void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        void waitIdle();

    private:
        void initDevice(const VuDeviceCreateInfo& info);

        void initVMA();

        void initCommandPool(const VuDeviceCreateInfo& info);

        void initBindlessDescriptorSetLayout(const VuDeviceCreateInfo& info);

        void initDescriptorPool(const VuDeviceCreateInfo& info);

        void initBindlessDescriptorSet();

        void initPipelineLayout();

        void initBindlessResourceManager(const VuDeviceCreateInfo& info);

        void initDefaultResources();
    };
}
