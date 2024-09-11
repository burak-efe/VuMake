#include "VuBuffer.h"

#include "VuRenderer.h"
#include "VuUtils.h"

VuBuffer::VuBuffer() {
}

VkResult VuBuffer::Init(VmaAllocator allocator,uint32 lenght,uint32 stride,VkBufferUsageFlagBits usage) {
    Allocator = allocator;
    Stride = stride;
    Lenght = lenght;
    VkBufferCreateInfo bufCreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufCreateInfo.size = lenght * stride;
    bufCreateInfo.usage = usage;

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo allocInfo;
    return vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &VulkanBuffer, &Allocation, &allocInfo);
}

void VuBuffer::Dispose() {
    vmaDestroyBuffer(Allocator, VulkanBuffer, Allocation);
}


VkResult VuBuffer::SetData(void *data, VkDeviceSize byteSize) {
    return vmaCopyMemoryToAllocation(Allocator, data, Allocation, 0, byteSize);
}



uint32 VuBuffer::findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties)  {
    VkPhysicalDeviceMemoryProperties memProperties{};
    vkGetPhysicalDeviceMemoryProperties(VuContext::PhysicalDevice, &memProperties);
    for (uint32 i = 0; i < memProperties.memoryTypeCount; i++) {
        if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


void VuBuffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                  VkMemoryPropertyFlags properties, VkBuffer &buffer,
                                  VkDeviceMemory &bufferMemory)  {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(VuContext::Device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(VuContext::Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(VuContext::Device, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(VuContext::Device, buffer, bufferMemory, 0);
}


void VuBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = VuRenderer::commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(VuContext::Device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(VuRenderer::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VuRenderer::graphicsQueue);
    vkFreeCommandBuffers(VuContext::Device, VuRenderer::commandPool, 1, &commandBuffer);
}