#pragma once

#include <span>
#include "10_Core/VuCommon.h"
#include "11_Config/VuConfig.h"
#include "VuTypes.h"
#include "VuPools.h"
#include "VuImage.h"
#include "VuBuffer.h"
#include "VuMaterialDataPool.h"
#include "VuSampler.h"
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

        VuPool2<VuImage, 32>   imagePool;
        VuPool2<VuSampler, 32> samplerPool;
        VuPool2<VuBuffer, 32>  bufferPool;
        VuPool2<VuShader, 32>  shaderPool;

        //holds the address of all other buffers
        VuBuffer            bdaBuffer;
        VuBuffer            stagingBuffer;
        VuHandle2<VuBuffer> debugBuffer{};
        VuMaterialDataPool  materialDataPool{};

        VuImage*   get(const VuHandle2<VuImage> handle) { return imagePool.get(handle); }
        VuSampler* get(const VuHandle2<VuSampler> handle) { return samplerPool.get(handle); }
        VuBuffer*  get(const VuHandle2<VuBuffer> handle) { return bufferPool.get(handle); }
        VuShader*  get(const VuHandle2<VuShader> handle) { return shaderPool.get(handle); }

        void destroyHandle(VuHandle2<VuImage> handle)
        {
            imagePool.destroyHandle(handle);
        }

        void destroyHandle(VuHandle2<VuSampler> handle)
        {
            samplerPool.destroyHandle(handle);
        }

        void destroyHandle(VuHandle2<VuBuffer> handle)
        {
            bufferPool.destroyHandle(handle);
        }

        void destroyHandle(VuHandle2<VuShader> handle)
        {
            shaderPool.destroyHandle(handle);
        }

        VuDisposeStack disposeStack;

        void uninit();

        void initInstance(VkBool32 enableValidation, std::span<const char*> validationLayers, std::span<const char*> instanceExtensions);

        void initDevice(const VuDeviceCreateInfo& info);

        void initVMA();

        void initCommandPool();

        void initBindlessDescriptor(const VuBindlessConfigInfo& info, uint32 maxFramesInFligth);

        void initDescriptorSetLayout(const VuBindlessConfigInfo& info);

        void initDescriptorPool(const VuBindlessConfigInfo& info);

        void initGlobalDescriptorSet(uint32 maxFramesInFlight);

        void initDefaultResources();

        VkCommandBuffer BeginSingleTimeCommands();

        void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

        VuHandle2<VuImage> createImage(const VuImageCreateInfo& info)
        {
            VuHandle2<VuImage> handle = imagePool.createHandle();
            VuImage*           image  = get(handle);
            assert(image != nullptr);
            image->init(device, memProperties, info);
            registerToBindless(image->imageView, handle.index);
            return handle;
        }

        VuHandle2<VuBuffer> createBuffer(const VuBufferCreateInfo& info)
        {
            VuHandle2<VuBuffer> handle = bufferPool.createHandle();
            VuBuffer*           buffer = get(handle);
            assert(buffer != nullptr);
            buffer->init(device, vma, info);
            registerToBindless(*buffer, handle.index);
            return handle;
        }

        VuHandle2<VuShader> createShader(Path vertexPath, Path fragPath, VkRenderPass renderPass)
        {
            VuHandle2<VuShader> handle = shaderPool.createHandle();
            VuShader*           shader = get(handle);
            assert(shader != nullptr);
            shader->init(this, vertexPath, fragPath, renderPass);
            return handle;
        }

        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
        {
            VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
            VkBufferCopy    copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size      = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
            EndSingleTimeCommands(commandBuffer);
        }

        void uploadToImage(const VuImage& vuImage, const byte* data, const VkDeviceSize size)
        {
            VkCheck(stagingBuffer.setData(data, size));

            VuImage::transitionImageLayout(*this, vuImage.image,
                                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            VuImage::copyBufferToImage(*this, stagingBuffer.buffer,
                                       vuImage.image,
                                       static_cast<uint32>(vuImage.lastCreateInfo.width),
                                       static_cast<uint32>(vuImage.lastCreateInfo.width));

            VuImage::transitionImageLayout(*this, vuImage.image,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        //Decriptor Management
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void initBindlessManager(const VuBindlessConfigInfo& info)
        {
            bdaBuffer.init(device, vma, {
                               .length = info.storageBufferCount,
                               .strideInBytes = sizeof(uint64)
                           });

            disposeStack.push([this]()-> void { bdaBuffer.uninit(); });
            registerBindlessBDA_Buffer(bdaBuffer);
        }

        //Register the buffer that holds BDA's of all other buffers
        void registerBindlessBDA_Buffer(const VuBuffer& buffer)
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = buffer.buffer;
            bufferInfo.range  = buffer.length * buffer.stride;

            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet          = globalDescriptorSets[i];
                descriptorWrite.dstBinding      = config::BINDLESS_CONFIG_INFO.storageBufferBinding;
                descriptorWrite.dstArrayElement = 0;
                descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pBufferInfo     = &bufferInfo;
                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }

        void writeUBO_ToGlobalPool(const VuBuffer& buffer, uint32 writeIndex, uint32 setIndex)
        {
            VkDescriptorBufferInfo bufferInfo{
                .buffer = buffer.buffer,
                .offset = 0,
                .range = sizeof(GPU_FrameConst)
            };

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = globalDescriptorSets[setIndex];
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &bufferInfo;
            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }

        void registerToBindless(const VuBuffer& buffer, uint32 bindlessIndex)
        {
            VkDeviceAddress address = buffer.getDeviceAddress();
            VkCheck(bdaBuffer.setData(&address, sizeof(VkDeviceAddress), bindlessIndex * sizeof(VkDeviceAddress)));
        }


        void registerToBindless(const VkImageView& imageView, uint32 bindlessIndex)
        {
            VkDescriptorImageInfo imageInfo{
                .sampler = VK_NULL_HANDLE,
                .imageView = imageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };

            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet          = globalDescriptorSets[i];
                descriptorWrite.dstBinding      = config::BINDLESS_CONFIG_INFO.sampledImageBinding;
                descriptorWrite.dstArrayElement = bindlessIndex;
                descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo      = &imageInfo;
                vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
            }
        }

        void registerToBindless(const VkSampler& sampler, uint32 bindlessIndex)
        {
            VkDescriptorImageInfo imageInfo{
                .sampler = sampler,
            };

            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkWriteDescriptorSet samplerWrite{};
                samplerWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                samplerWrite.dstSet          = globalDescriptorSets[i];
                samplerWrite.dstBinding      = config::BINDLESS_CONFIG_INFO.samplerBinding;
                samplerWrite.dstArrayElement = bindlessIndex;
                samplerWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
                samplerWrite.descriptorCount = 1;
                samplerWrite.pImageInfo      = &imageInfo;
                vkUpdateDescriptorSets(device, 1, &samplerWrite, 0, nullptr);
            }
        }


        //STATIC FUNCTIONS
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

        static bool isDeviceSupportExtensions(VkPhysicalDevice device, VkSurfaceKHR surface, std::span<const char*> enabledExtensions);

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
