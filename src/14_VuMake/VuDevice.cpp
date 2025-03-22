#include "VuDevice.h"

#include <iostream>
#include <set>

#include "10_Core/Color32.h"
#include "12_VuMakeCore/VuCreateUtils.h"

Vu::VuImage* Vu::VuDevice::getImage(const VuHnd<VuImage> handle)
{
    return imagePool.getResource(handle);
}

Vu::VuSampler* Vu::VuDevice::getSampler(const VuHnd<VuSampler> handle)
{
    return samplerPool.getResource(handle);
}

Vu::VuBuffer* Vu::VuDevice::getBuffer(const VuHnd<VuBuffer> handle)
{
    return bufferPool.getResource(handle);
}

Vu::VuShader* Vu::VuDevice::getShader(const VuHnd<VuShader> handle)
{
    return shaderPool.getResource(handle);
}

Vu::VuMaterial* Vu::VuDevice::getMaterial(const VuHnd<VuMaterial> handle)
{
    return materialPool.getResource(handle);
}

Vu::uint32* Vu::VuDevice::getMaterialDataIndex(const VuHnd<uint32> handle)
{
    return materialDataIndexPool.getResource(handle);
}

void Vu::VuDevice::destroyHandle(VuHnd<VuImage> handle)
{
    imagePool.destroyHandle(handle);
}

void Vu::VuDevice::destroyHandle(VuHnd<VuSampler> handle)
{
    samplerPool.destroyHandle(handle);
}

void Vu::VuDevice::destroyHandle(VuHnd<VuBuffer> handle)
{
    bufferPool.destroyHandle(handle);
}

void Vu::VuDevice::destroyHandle(VuHnd<VuShader> handle)
{
    shaderPool.destroyHandle(handle);
}

void Vu::VuDevice::destroyHandle(VuHnd<VuMaterial> handle)
{
    materialPool.destroyHandle(handle);
}

void Vu::VuDevice::destroyHandle(VuHnd<uint32> handle)
{
    materialDataIndexPool.destroyHandle(handle);
}

void Vu::VuDevice::registerBindlessBDA_Buffer(const VuBuffer& buffer)
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

void Vu::VuDevice::writeUBO_ToGlobalPool(const VuBuffer& buffer, uint32 writeIndex, uint32 setIndex)
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

void Vu::VuDevice::registerToBindless(const VuBuffer& buffer, uint32 bindlessIndex)
{
    VkDeviceAddress address = buffer.getDeviceAddress();
    VkCheck(bdaBuffer.setData(&address, sizeof(VkDeviceAddress), bindlessIndex * sizeof(VkDeviceAddress)));
}

void Vu::VuDevice::registerToBindless(const VkImageView& imageView, uint32 bindlessIndex)
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

void Vu::VuDevice::registerToBindless(const VkSampler& sampler, uint32 bindlessIndex)
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

void Vu::VuDevice::uninit()
{
    disposeStack.disposeAll();
    std::cout << "VuDevice::uninit()" << std::endl;
    std::cout << imagePool.getUsedSlotCount() << std::endl;
    std::cout << samplerPool.getUsedSlotCount() << std::endl;
    std::cout << bufferPool.getUsedSlotCount() << std::endl;
    std::cout << shaderPool.getUsedSlotCount() << std::endl;
    std::cout << materialPool.getUsedSlotCount() << std::endl;
    std::cout << materialDataIndexPool.getUsedSlotCount() << std::endl;
}

void Vu::VuDevice::initInstance(VkBool32               enableValidation,
                                std::span<const char*> validationLayers,
                                std::span<const char*> instanceExtensions)
{
    CreateUtils::createInstance(enableValidation, validationLayers, instanceExtensions, instance);
    disposeStack.push([&]
    {
        vkDestroyInstance(instance, nullptr);
    });

    if (enableValidation)
    {
        CreateUtils::createDebugMessenger(instance, debugMessenger);
        disposeStack.push([&]
        {
            CreateUtils::destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        });
    }
}

void Vu::VuDevice::initDevice(const VuDeviceCreateInfo& info)
{
    CreateUtils::createPhysicalDevice(instance, info.surface, info.deviceExtensions, physicalDevice);
    queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(physicalDevice, info.surface);

    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    CreateUtils::createDevice(info.physicalDeviceFeatures2,
                              queueFamilyIndices,
                              physicalDevice,
                              info.deviceExtensions,
                              device, graphicsQueue, presentQueue
                             );
    disposeStack.push([&] { vkDestroyDevice(device, nullptr); });

    initVMA();

    initCommandPool();
}

void Vu::VuDevice::initVMA()
{
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
    disposeStack.push([&] { vmaDestroyAllocator(vma); });
}

void Vu::VuDevice::initCommandPool()
{
    ZoneScoped;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    VkCheck(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
    disposeStack.push([&] { vkDestroyCommandPool(device, commandPool, nullptr); });
}

void Vu::VuDevice::initBindlessDescriptor(const VuBindlessConfigInfo& info, uint32 maxFramesInFlight)
{
    initDescriptorSetLayout(info);
    initDescriptorPool(info);
    initGlobalDescriptorSet(maxFramesInFlight);

    std::array descSetLayouts{globalDescriptorSetLayout};
    CreateUtils::createPipelineLayout(device, descSetLayouts, config::PUSH_CONST_SIZE, globalPipelineLayout);
    disposeStack.push([&] { vkDestroyPipelineLayout(device, globalPipelineLayout, nullptr); });
}

void Vu::VuDevice::initDescriptorSetLayout(const VuBindlessConfigInfo& info)
{
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

    std::array descriptorSetLayoutFlags{flag, flag, flag, flag, flag};

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
    binding_flags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    binding_flags.bindingCount  = descriptorSetLayoutFlags.size();
    binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();
    globalSetLayout.pNext       = &binding_flags;

    VkCheck(vkCreateDescriptorSetLayout(device, &globalSetLayout, nullptr, &globalDescriptorSetLayout));
    disposeStack.push([&] { vkDestroyDescriptorSetLayout(device, globalDescriptorSetLayout, nullptr); });
}

void Vu::VuDevice::initDefaultResources()
{
    stagingBuffer.init(device, vma, {
                           .name = "stagingBuffer",
                           .length = 1024 * 1024 * 64,
                           .vkUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           .vmaCreateFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
                       });
    disposeStack.push([&] { stagingBuffer.uninit(); });
    stagingBuffer.map();

    debugBufferHnd = createBuffer({
                                      .name = "defaulttBuffer",
                                      .length = 4096,
                                      .strideInBytes = 1,
                                      .vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                      .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
                                      .vmaCreateFlags = 0
                                  });
    disposeStack.push([&] { destroyHandle(debugBufferHnd); });
    assert(debugBufferHnd.index == 0);

    Vector<Color32> colorData;
    Color32         defaultColor = Color32(0.0f, 0.0f, 1.0f);
    colorData.resize(512 * 512, defaultColor);

    defaultImageHandle = createImage({.width = 512, .height = 512, .format = VK_FORMAT_R8G8B8A8_SRGB});
    disposeStack.push([&] { destroyHandle(defaultImageHandle); });
    uploadToImage(*getImage(defaultImageHandle), reinterpret_cast<const byte*>(colorData.data()), colorData.size() * sizeof(Color32));
    assert(defaultImageHandle.index == 0);

    Color32 normalColor = Color32(uint8_t(128), uint8_t(128), uint8_t(255), uint8_t(255));
    std::fill(colorData.begin(), colorData.end(), normalColor);
    defaultNormalImageHandle = createImage({.width = 512, .height = 512, .format = VK_FORMAT_R8G8B8A8_UNORM});
    disposeStack.push([&] { destroyHandle(defaultNormalImageHandle); });
    uploadToImage(*getImage(defaultNormalImageHandle), reinterpret_cast<const byte*>(colorData.data()), colorData.size() * sizeof(Color32));
    assert(defaultNormalImageHandle.index == 1);

    //TODO pass physical props max
    defaultSamplerHandle = createSampler({.maxAnisotropy = 16.0f});
    disposeStack.push([&] { destroyHandle(defaultSamplerHandle); });
    assert(defaultSamplerHandle.index == 0);

    materialDataBufferHandle = createBuffer({
                                                .name = "materialDataBuffer",
                                                .length = config::MAX_MATERIAL_DATA,
                                                .strideInBytes = config::MATERIAL_DATA_SIZE,
                                                .vkUsageFlags =
                                                VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
                                                .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                  VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                            });
    disposeStack.push([&] { destroyHandle(materialDataBufferHandle); });
    assert(materialDataBufferHandle.index == 1);
    VuBuffer* matDataBuffer = getBuffer(materialDataBufferHandle);
    matDataBuffer->map();
}

void Vu::VuDevice::initDescriptorPool(const VuBindlessConfigInfo& info)
{
    ZoneScoped;

    std::array<VkDescriptorPoolSize, 5> poolSizes{
        {
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = info.uboCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = info.samplerCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = info.sampledImageCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = info.storageImageCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = info.storageBufferCount * config::MAX_FRAMES_IN_FLIGHT},
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

void Vu::VuDevice::initGlobalDescriptorSet(uint32 maxFramesInFlight)
{
    ZoneScoped;
    std::vector globalLayouts(maxFramesInFlight, globalDescriptorSetLayout);

    VkDescriptorSetAllocateInfo globalSetsAllocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = globalLayouts.data(),
    };

    globalDescriptorSets.resize(maxFramesInFlight);
    VkCheck(vkAllocateDescriptorSets(device, &globalSetsAllocInfo, globalDescriptorSets.data()));
    disposeStack.push([&] { vkDestroyDescriptorPool(device, descriptorPool, nullptr); });
}

VkCommandBuffer Vu::VuDevice::BeginSingleTimeCommands()
{
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

void Vu::VuDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

Vu::VuHnd<Vu::VuImage> Vu::VuDevice::createImage(const VuImageCreateInfo& info)
{
    VuHnd<VuImage> handle = imagePool.createHandle();
    VuImage*       image  = getImage(handle);
    assert(image != nullptr);
    image->init(device, memProperties, info);
    registerToBindless(image->imageView, handle.index);
    return handle;
}

Vu::VuHnd<Vu::VuBuffer> Vu::VuDevice::createBuffer(const VuBufferCreateInfo& info)
{
    VuHnd<VuBuffer> handle = bufferPool.createHandle();
    VuBuffer*       buffer = getBuffer(handle);
    assert(buffer != nullptr);
    buffer->init(device, vma, info);
    registerToBindless(*buffer, handle.index);
    return handle;
}

Vu::VuHnd<Vu::VuShader> Vu::VuDevice::createShader(Path vertexPath, Path fragPath, VkRenderPass renderPass)
{
    VuHnd<VuShader> handle   = shaderPool.createHandle();
    VuShader*       resource = getShader(handle);
    assert(resource != nullptr);
    resource->init(this, vertexPath, fragPath, renderPass);
    return handle;
}

Vu::VuHnd<unsigned> Vu::VuDevice::createMaterialDataIndex()
{
    VuHnd<uint32> handle   = materialDataIndexPool.createHandle();
    uint32*       resource = getMaterialDataIndex(handle);
    assert(resource != nullptr);
    *resource = handle.index;
    return handle;
}

Vu::VuHnd<Vu::VuMaterial> Vu::VuDevice::createMaterial(MaterialSettings matSettings, VuHnd<VuShader> shaderHnd,
                                                       VuHnd<uint32>    materialDataHnd)
{
    VuHnd<VuMaterial> handle   = materialPool.createHandle();
    VuMaterial*       resource = getMaterial(handle);
    assert(resource != nullptr);
    resource->init(this, matSettings, shaderHnd, materialDataHnd);
    return handle;
}

Vu::GPU_PBR_MaterialData* Vu::VuDevice::getMaterialData(VuHnd<uint32> handle)
{
    VuBuffer* matDataBuffer = getBuffer(materialDataBufferHandle);
    byte*     dataPtr       = static_cast<byte*>(matDataBuffer->mapPtr) + config::MATERIAL_DATA_SIZE * handle.index;
    return reinterpret_cast<GPU_PBR_MaterialData*>(dataPtr);
}

void Vu::VuDevice::bindMaterial(VkCommandBuffer cb, VuHnd<VuMaterial> material)
{
    auto*              mat      = getMaterial(material);
    auto*              shader   = getShader(mat->shaderHnd);
    VuGraphicsPipeline pipeline = shader->requestPipeline(mat->materialSettings);

    vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
}

Vu::VuHnd<Vu::VuImage> Vu::VuDevice::createImageFromAsset(const Path& path, VkFormat format)
{
    ZoneScoped;
    int   texWidth;
    int   texHeight;
    int   texChannels;
    byte* pixels;

    Vu::VuImage::loadImageFile(path, texWidth, texHeight, texChannels, reinterpret_cast<stbi_uc*&>(pixels));
    const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4U);

    if (pixels == nullptr)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    uint32         w      = texWidth;
    uint32         h      = texHeight;
    VuHnd<VuImage> handle = createImage({.width = w, .height = h, .format = format});
    uploadToImage(*getImage(handle), pixels, imageSize);
    stbi_image_free(pixels);
    return handle;
}

Vu::VuHnd<Vu::VuSampler> Vu::VuDevice::createSampler(const VuSamplerCreateInfo& info)
{
    VuHnd<VuSampler> handle   = samplerPool.createHandle();
    VuSampler*       resource = getSampler(handle);
    assert(resource != nullptr);
    resource->init(device, info);
    registerToBindless(resource->vkSampler, handle.index);
    return handle;
}

void Vu::VuDevice::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferCopy    copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    EndSingleTimeCommands(commandBuffer);
}

void Vu::VuDevice::uploadToImage(const VuImage& vuImage, const byte* data, const VkDeviceSize size)
{
    VkCheck(stagingBuffer.setData(data, size));

    transitionImageLayout(vuImage.image,
                          VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(stagingBuffer.buffer,
                      vuImage.image,
                      vuImage.lastCreateInfo.width,
                      vuImage.lastCreateInfo.width);

    transitionImageLayout(vuImage.image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                          VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void Vu::VuDevice::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer      commandBuffer = BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
                         commandBuffer,
                         sourceStage, destinationStage,
                         0,
                         0, nullptr,
                         0, nullptr,
                         1, &barrier
                        );

    EndSingleTimeCommands(commandBuffer);
}

void Vu::VuDevice::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

    VkBufferImageCopy region{};
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

void Vu::VuDevice::initBindlessManager(const VuBindlessConfigInfo& info)
{
    bdaBuffer.init(device, vma, {
                       .name = "BDA_Buffer",
                       .length = info.storageBufferCount,
                       .strideInBytes = sizeof(uint64)
                   });

    disposeStack.push([this]()-> void { bdaBuffer.uninit(); });
    registerBindlessBDA_Buffer(bdaBuffer);
}
