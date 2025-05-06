#pragma once

#include <cstdint>                        // for uint32_t
#include <functional>                     // for function
#include <memory>                         // for shared_ptr
#include <optional>                       // for optional
#include <span>                           // for span
#include <stack>                          // for stack
#include <vector>                         // for vector

#include <vulkan/vulkan_core.h>           // for VkBuffer, VkCommandBuffer
#include "vk_mem_alloc.h"                 // for VmaAllocator

#include "08_LangUtils/IndexAllocator.h"  // for IndexAllocator
#include "08_LangUtils/TypeDefs.h"        // for u32, byte
#include "10_Core/VuCommon.h"             // for path
#include "12_VuMakeCore/VuBuffer.h"       // for VuBuffer, VuBufferCreateInf...
#include "12_VuMakeCore/VuImage.h"        // for VuImage, VuImageCreateInfo ...
#include "12_VuMakeCore/VuSampler.h"      // for VuSampler, VuSamplerCreateI...
#include "12_VuMakeCore/VuTypes.h"        // for QueueFamilyIndices, VuDispo...
#include "14_VuMake/VuMaterial.h"         // for MaterialSettings (ptr only)
#include "VuShader.h"                     // for VuShader

namespace Vu { struct VuRenderPass; }

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

    };

    struct VuDevice
    {
        VkInstance       instance{};
        VkPhysicalDevice physicalDevice{};

        QueueFamilyIndices               queueFamilyIndices{};
        VkDevice                         device{};
        VkQueue                          graphicsQueue{};
        VkQueue                          presentQueue{};
        VkCommandPool                    commandPool{};
        VkDescriptorSetLayout            globalDescriptorSetLayout{};
        std::vector<VkDescriptorSet>     globalDescriptorSets{};
        VkDescriptorPool                 descriptorPool{};
        VkDescriptorPool                 uiDescriptorPool{};
        VkPipelineLayout                 globalPipelineLayout{};
        VmaAllocator                     vma{};
        VkPhysicalDeviceMemoryProperties memProperties{};
        IndexAllocator                   imgBindlessIndexAllocator{};
        IndexAllocator                   samplerBindlessIndexAllocator{};
        IndexAllocator                   bufferBindlessIndexAllocator{};
        IndexAllocator                   materialDataBindlessIndexAllocator{};

    private:
        //holds the address of all other buffers
        VuBuffer                   bdaBuffer{};
        VuBuffer                   stagingBuffer{};
        std::shared_ptr<VuBuffer>  debugBufferHnd{};
        std::shared_ptr<VuBuffer>  materialDataBufferHandle{};
        std::shared_ptr<VuImage>   defaultImageHandle{};
        std::shared_ptr<VuImage>   defaultNormalImageHandle{};
        std::shared_ptr<VuSampler> defaultSamplerHandle{};
        VuDisposeStack             disposeStack{};

    public:
        //RESOURCES

        VuDevice() = default;
        explicit                   VuDevice(const VuDeviceCreateInfo& createInfo);
        std::shared_ptr<VuImage>   createImage(const VuImageCreateInfo& info);
        std::shared_ptr<VuImage>   createImageFromAsset(const path& path, VkFormat format);
        std::shared_ptr<VuSampler> createSampler(const VuSamplerCreateInfo& info);
        std::shared_ptr<VuBuffer>  createBuffer(const VuBufferCreateInfo& info);
        std::shared_ptr<VuShader>  createShader(path vertexPath, path fragPath, VuRenderPass* vuRenderPass);

        std::shared_ptr<u32> createMaterialDataIndex();

        std::shared_ptr<VuMaterial> createMaterial(MaterialSettings matSettings, std::shared_ptr<VuShader> shaderHnd,
                                                   std::shared_ptr<u32> materialDataHnd);

        std::span<byte, 64> getMaterialData(std::shared_ptr<u32> handle);

        void bindMaterial(VkCommandBuffer cb, std::shared_ptr<VuMaterial> material);

        void uninit();

        void registerBindlessBDA_Buffer(const VuBuffer& buffer) const;

        void writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const;

        void registerToBindless(const VuBuffer& buffer, u32 bindlessIndex) const;

        void registerToBindless(const VkImageView& imageView, u32 bindlessIndex) const;

        void registerToBindless(const VkSampler& sampler, u32 bindlessIndex) const;

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
