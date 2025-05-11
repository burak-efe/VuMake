#include "VuDevice.h"

#include <algorithm>                           // for fill
#include <array>                               // for array
#include <cassert>                             // for assert
#include <cstddef>                             // for size_t, byte
#include <memory_resource>                     // for new_delete_resource
#include <stdexcept>                           // for invalid_argument, runt...

#include "03_Mantle/VuBuffer.h"
#include "stb_image.h"

namespace Vu
{
struct VuRenderPass;
}

void Vu::VuDevice::registerBindlessBDA_Buffer(const VuBuffer& buffer) const
{
    vk::DescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer.buffer;
    bufferInfo.range  = buffer.sizeInBytes;

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::WriteDescriptorSet descriptorWrite{};
        descriptorWrite.dstSet          = globalDescriptorSets[i];
        descriptorWrite.dstBinding      = config::BINDLESS_STORAGE_BUFFER_BINDING;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType  = vk::DescriptorType::eStorageBuffer;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo     = &bufferInfo;

        device.updateDescriptorSets(descriptorWrite, {});
    }
}

void Vu::VuDevice::writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const
{
    vk::DescriptorBufferInfo bufferInfo{
            .buffer{buffer.buffer},
            .offset{0},
            .range{sizeof(GPU_FrameConst)}
    };

    vk::WriteDescriptorSet descriptorWrite{};
    descriptorWrite.dstSet          = globalDescriptorSets[setIndex];
    descriptorWrite.dstBinding      = 0;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = vk::DescriptorType::eUniformBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo     = &bufferInfo;
    device.updateDescriptorSets(descriptorWrite, {});
}

void Vu::VuDevice::registerToBindless(const VuBuffer& buffer, u32 bindlessIndex) const
{
    vk::DeviceAddress address = buffer.getDeviceAddress();
    //TODO handle error
    auto res = bdaBuffer.setData(&address, sizeof(vk::DeviceAddress), bindlessIndex * sizeof(vk::DeviceAddress));
}

void Vu::VuDevice::registerToBindless(const vk::ImageView& imageView, u32 bindlessIndex) const
{
    vk::DescriptorImageInfo imageInfo{
            .sampler = VK_NULL_HANDLE,
            .imageView = imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::WriteDescriptorSet descriptorWrite{};
        descriptorWrite.dstSet          = globalDescriptorSets[i];
        descriptorWrite.dstBinding      = config::BINDLESS_SAMPLED_IMAGE_BINDING;
        descriptorWrite.dstArrayElement = bindlessIndex;
        descriptorWrite.descriptorType  = vk::DescriptorType::eSampledImage;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo      = &imageInfo;
        device.updateDescriptorSets(descriptorWrite, {});
    }
}

void Vu::VuDevice::registerToBindless(const vk::Sampler& sampler, u32 bindlessIndex) const
{
    vk::DescriptorImageInfo imageInfo{
            .sampler = sampler,
    };

    for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vk::WriteDescriptorSet descriptorWrite{};
        descriptorWrite.dstSet          = globalDescriptorSets[i];
        descriptorWrite.dstBinding      = config::BINDLESS_SAMPLER_BINDING;
        descriptorWrite.dstArrayElement = bindlessIndex;
        descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo      = &imageInfo;
        device.updateDescriptorSets(descriptorWrite, {});
    }
}

void Vu::VuDevice::uninit()
{
    disposeStack.disposeAll();
}


void Vu::VuDevice::initDevice(const VuDeviceCreateInfo& info)
{


    this->instance       = info.instance;
    this->physicalDevice = info.physicalDevice;

    queueFamilyIndices = VuQueueFamilyIndices::findQueueFamilies(physicalDevice, info.surface);

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    CreateUtils::createDevice(info.requestedFeatures,
                              queueFamilyIndices,
                              physicalDevice,
                              info.deviceExtensions,
                              device, graphicsQueue, presentQueue
            );
    disposeStack.push([&] { vkDestroyDevice(device, nullptr); });
}


void Vu::VuDevice::initCommandPool(const VuDeviceCreateInfo& info)
{
    vk::CommandPoolCreateInfo poolInfo{};
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    //Todo
    auto res = vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
    disposeStack.push([&] { vkDestroyCommandPool(device, commandPool, nullptr); });
}

void Vu::VuDevice::initPipelineLayout()
{
    std::array descSetLayouts{globalDescriptorSetLayout};
    CreateUtils::createPipelineLayout(device, descSetLayouts, config::PUSH_CONST_SIZE, globalPipelineLayout);
    disposeStack.push([&] { vkDestroyPipelineLayout(device, globalPipelineLayout, nullptr); });
}

void Vu::VuDevice::initBindlessDescriptorSetLayout(const VuDeviceCreateInfo& info)
{
    vk::DescriptorSetLayoutBinding ubo{
            .binding = info.uboBinding,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = info.uboCount,
            .stageFlags = VK_SHADER_STAGE_ALL,
    };
    vk::DescriptorSetLayoutBinding sampler{
            .binding = info.samplerBinding,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = info.samplerCount,
            .stageFlags = VK_SHADER_STAGE_ALL,
    };
    vk::DescriptorSetLayoutBinding sampledImage{
            .binding = info.sampledImageBinding,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = info.sampledImageCount,
            .stageFlags = VK_SHADER_STAGE_ALL,
    };
    vk::DescriptorSetLayoutBinding storageImage{
            .binding = info.storageImageBinding,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            .descriptorCount = info.storageImageCount,
            .stageFlags = VK_SHADER_STAGE_ALL,
    };

    vk::DescriptorSetLayoutBinding storageBuffer{
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
            storageBuffer,
    };

    vk::DescriptorSetLayoutCreateInfo globalSetLayout{
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
            .bindingCount = descriptorSetLayoutBindings.size(),
            .pBindings = descriptorSetLayoutBindings.data(),
    };

    const vk::DescriptorBindingFlagsEXT flag =
            vk::DescriptorBindingFlagBitsEXT::ePartiallyBound |
            vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind |
            vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;


    std::array descriptorSetLayoutFlags{flag, flag, flag, flag, flag};

    vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
    binding_flags.bindingCount  = descriptorSetLayoutFlags.size();
    binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();
    globalSetLayout.pNext       = &binding_flags;

    vk::Check(vkCreateDescriptorSetLayout(device, &globalSetLayout, nullptr, &globalDescriptorSetLayout));
    disposeStack.push([&] { vkDestroyDescriptorSetLayout(device, globalDescriptorSetLayout, nullptr); });
}

void Vu::VuDevice::initDefaultResources()
{
    stagingBuffer.init(device, vma, {
                               .name = "stagingBuffer",
                               .length = 1024 * 1024 * 64,
                               .vkUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                               .vmaCreateFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                 VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                       });
    disposeStack.push([&] { stagingBuffer.uninit(); });
    stagingBuffer.map();

    debugBufferHnd = createBuffer({
            .name = "defaulttBuffer",
            .length = 4096,
            .strideInBytes = 1,
            .vkUsageFlags =
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
            .vmaCreateFlags = 0
    });

    //assert(debugBufferHnd.index == 0);
    std::vector<Color32> colorData;
    Color32              defaultColor = Color32(0.0f, 0.0f, 0.0f);
    Color32              magentaColor = Color32(1.0f, 0.0f, 1.0f);
    colorData.resize(512 * 512);

    for (int y = 0; y < 512; ++y)
    {
        for (int x = 0; x < 512; ++x)
        {
            // Determine the block (x // blockSize, y // blockSize)
            bool isMagenta         = ((x / 16) + (y / 16)) % 2 == 0;
            colorData[y * 512 + x] = isMagenta ? magentaColor : defaultColor;
        }
    }

    defaultImageHandle = createImage({.width = 512, .height = 512, .format = vk::Format::eR8G8B8A8Srgb});
    uploadToImage(*defaultImageHandle.get(), reinterpret_cast<const byte*>(colorData.data()),
                  colorData.size() * sizeof(Color32));
    //assert(defaultImageHandle.index == 0);

    Color32 normalColor = Color32(uint8_t(128), uint8_t(128), uint8_t(255), uint8_t(255));
    std::fill(colorData.begin(), colorData.end(), normalColor);
    defaultNormalImageHandle = createImage({.width = 512, .height = 512, .format = vk::Format::eR8G8B8A8Unorm});

    uploadToImage(*defaultNormalImageHandle.get(), reinterpret_cast<const byte*>(colorData.data()),
                  colorData.size() * sizeof(Color32));
    //assert(defaultNormalImageHandle.index == 1);

    //TODO pass physical props max
    defaultSamplerHandle = createSampler({.maxAnisotropy = 16.0f});

    //assert(defaultSamplerHandle.get(). == 0);

    materialDataBufferHandle = createBuffer({
            .name = "materialDataBuffer",
            .length = config::DEVICE_MAX_MATERIAL_DATA_COUNT,
            .strideInBytes = config::MATERIAL_DATA_SIZE,
            .vkUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
                            | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
            .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
            .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                              | VMA_ALLOCATION_CREATE_MAPPED_BIT,
    });

    assert(materialDataBufferHandle.get()->bindlessIndex == 1);
    VuBuffer* matDataBuffer = materialDataBufferHandle.get();
    matDataBuffer->map();
}

void Vu::VuDevice::initDescriptorPool(const VuDeviceCreateInfo& info)
{
    std::array<vk::DescriptorPoolSize, 5> poolSizes{
            {
                    {
                            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = info.uboCount * config::MAX_FRAMES_IN_FLIGHT
                    },
                    {.type = VK_DESCRIPTOR_TYPE_SAMPLER,
                     .descriptorCount = info.samplerCount * config::MAX_FRAMES_IN_FLIGHT},
                    {
                            .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                            .descriptorCount = info.sampledImageCount * config::MAX_FRAMES_IN_FLIGHT
                    },
                    {
                            .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                            .descriptorCount = info.storageImageCount * config::MAX_FRAMES_IN_FLIGHT
                    },
                    {
                            .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                            .descriptorCount = info.storageBufferCount * config::MAX_FRAMES_IN_FLIGHT
                    },
            },
    };

    vk::DescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 2,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
    };
    vk::Check(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
}

void Vu::VuDevice::initBindlessDescriptorSet()
{
    std::array<vk::DescriptorSetLayout, config::MAX_FRAMES_IN_FLIGHT> globalDescLayout;
    globalDescLayout.fill(globalDescriptorSetLayout);

    vk::DescriptorSetAllocateInfo globalSetsAllocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = globalDescLayout.size(),
            .pSetLayouts = globalDescLayout.data(),
    };

    globalDescriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
    vk::Check(vkAllocateDescriptorSets(device, &globalSetsAllocInfo, globalDescriptorSets.data()));
    disposeStack.push([&] { vkDestroyDescriptorPool(device, descriptorPool, nullptr); });
}

vk::CommandBuffer Vu::VuDevice::BeginSingleTimeCommands()
{
    vk::CommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = commandPool;
    allocInfo.commandBufferCount = 1;

    vk::CommandBuffer commandBuffer;
    vk::Check(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Vu::VuDevice::EndSingleTimeCommands(vk::CommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    vk::SubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);

}

std::expected<Vu::VuDevice, vk::Result> Vu::VuDevice::make(const VuDeviceCreateInfo& createInfo)
{
    try
    {
        Vu::VuDevice device{createInfo};
        return device;
    }
    catch (vk::Result res)
    {
        return std::unexpected{res};
    }

}

Vu::VuDevice::VuDevice(const VuDeviceCreateInfo& info) :
    imgBindlessIndexAllocator{info.sampledImageCount, std::pmr::new_delete_resource()},
    samplerBindlessIndexAllocator{info.samplerCount, std::pmr::new_delete_resource()},
    bufferBindlessIndexAllocator{info.storageBufferCount, std::pmr::new_delete_resource()},
    materialDataBindlessIndexAllocator{1024, std::pmr::new_delete_resource()}
{
    initDevice(info);
    initCommandPool(info);
    initBindlessDescriptorSetLayout(info);
    initDescriptorPool(info);
    initPipelineLayout();
    initBindlessDescriptorSet();
    initBindlessResourceManager(info);
    initDefaultResources();
}

std::shared_ptr<Vu::VuImage> Vu::VuDevice::createImage(const VuImageCreateInfo& info)
{
    std::shared_ptr<VuImage> handle   = std::make_shared<VuImage>();
    VuImage*                 resource = handle.get();
    resource->init(device, memProperties, info);
    u32 bindlessIndex = imgBindlessIndexAllocator.allocate();
    registerToBindless(resource->imageView, bindlessIndex);
    resource->bindlessIndex = bindlessIndex;
    return handle;
}

std::shared_ptr<Vu::VuImage> Vu::VuDevice::createImageFromAsset(const path& path, vk::Format format)
{
    int   texWidth;
    int   texHeight;
    int   texChannels;
    byte* pixels;

    Vu::VuImage::loadImageFile(path, texWidth, texHeight, texChannels, reinterpret_cast<stbi_uc*&>(pixels));
    const auto imageSize = static_cast<vk::DeviceSize>(texWidth * texHeight * 4U);

    if (pixels == nullptr)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    u32                      w      = texWidth;
    u32                      h      = texHeight;
    std::shared_ptr<VuImage> handle = createImage({.width = w, .height = h, .format = format});
    uploadToImage(*handle.get(), pixels, imageSize);
    stbi_image_free(pixels);
    return handle;
}

std::shared_ptr<Vu::VuSampler> Vu::VuDevice::createSampler(const VuSamplerCreateInfo& info)
{
    std::shared_ptr<VuSampler> handle   = std::make_shared<VuSampler>();
    VuSampler*                 resource = handle.get();

    resource->init(device, info);
    uint32_t bindlessIndex = samplerBindlessIndexAllocator.allocate();
    registerToBindless(resource->sampler, bindlessIndex);
    resource->bindlessIndex = bindlessIndex;
    return handle;
}

std::shared_ptr<Vu::VuBuffer> Vu::VuDevice::createBuffer(const VuBufferCreateInfo& info)
{
    std::shared_ptr<VuBuffer> handle   = std::make_shared<VuBuffer>();
    VuBuffer*                 resource = handle.get();

    resource->init(device, vma, info);
    u32 bindlessIndex = bufferBindlessIndexAllocator.allocate();
    registerToBindless(*resource, bindlessIndex);
    resource->bindlessIndex = bindlessIndex;
    return handle;
}

std::shared_ptr<Vu::VuShader> Vu::VuDevice::createShader(path vertexPath, path fragPath, VuRenderPass* vuRenderPass)
{
    std::shared_ptr<VuShader> handle   = std::make_shared<VuShader>();
    VuShader*                 resource = handle.get();
    resource->init(this, vertexPath, fragPath, vuRenderPass);
    return handle;
}

std::shared_ptr<unsigned> Vu::VuDevice::createMaterialDataIndex()
{
    std::shared_ptr<u32> handle        = std::make_shared<u32>();
    u32*                 resource      = handle.get();
    u32                  bindlessIndex = materialDataBindlessIndexAllocator.allocate();
    *resource                          = bindlessIndex;
    return handle;
}

std::shared_ptr<Vu::VuMaterial> Vu::VuDevice::createMaterial(MaterialSettings          matSettings,
                                                             std::shared_ptr<VuShader> shaderHnd,
                                                             std::shared_ptr<u32>      materialDataHnd)
{
    std::shared_ptr<VuMaterial> handle   = std::make_shared<VuMaterial>();
    VuMaterial*                 resource = handle.get();
    assert(resource != nullptr);
    *resource = VuMaterial{this, matSettings, shaderHnd, materialDataHnd};
    return handle;
}

std::span<std::byte, Vu::config::MATERIAL_DATA_SIZE> Vu::VuDevice::getMaterialData(std::shared_ptr<u32> handle)
{
    VuBuffer* matDataBuffer = materialDataBufferHandle.get();
    byte*     dataPtr       = static_cast<byte*>(matDataBuffer->mapPtr) + config::MATERIAL_DATA_SIZE * *handle;
    return std::span<std::byte, Vu::config::MATERIAL_DATA_SIZE>(dataPtr, config::MATERIAL_DATA_SIZE);
}

void Vu::VuDevice::bindMaterial(vk::CommandBuffer cb, std::shared_ptr<VuMaterial> material)
{
    auto*              mat      = material.get();
    auto*              shader   = mat->shaderHnd.get();
    VuGraphicsPipeline pipeline = shader->requestPipeline(mat->materialSettings);
    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
}


void Vu::VuDevice::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();
    vk::BufferCopy    copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
    EndSingleTimeCommands(commandBuffer);
}

void Vu::VuDevice::uploadToImage(const VuImage& vuImage, const byte* data, const vk::DeviceSize size)
{
    //todo
    auto res = stagingBuffer.setData(data, size);

    transitionImageLayout(vuImage.image,
                          vk::ImageLayout::eUndefined,
                          vk::ImageLayout::eTransferDstOptimal);

    copyBufferToImage(stagingBuffer.buffer,
                      vuImage.image,
                      vuImage.lastCreateInfo.width,
                      vuImage.lastCreateInfo.height);

    transitionImageLayout(vuImage.image,
                          vk::ImageLayout::eTransferDstOptimal,
                          vk::ImageLayout::eShaderReadOnlyOptimal);
}


void Vu::VuDevice::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage      = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    commandBuffer.pipelineBarrier(
            sourceStage, destinationStage,
            {},
            nullptr,
            nullptr,
            barrier
            );

    EndSingleTimeCommands(commandBuffer);
}


void Vu::VuDevice::waitIdle()
{
    Logger::Trace("Wait Idle Called!");
    vkDeviceWaitIdle(device);
}

void Vu::VuDevice::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::CommandBuffer commandBuffer = BeginSingleTimeCommands();

    vk::BufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {
            width,
            height,
            1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndSingleTimeCommands(commandBuffer);
}

void Vu::VuDevice::initBindlessResourceManager(const VuDeviceCreateInfo& info)
{
    bdaBuffer.init(device, vma, {
                           .name = "BDA_Buffer",
                           .length = info.storageBufferCount,
                           .strideInBytes = sizeof(u64)
                   });

    disposeStack.push([this]()-> void { bdaBuffer.uninit(); });
    registerBindlessBDA_Buffer(bdaBuffer);
}
