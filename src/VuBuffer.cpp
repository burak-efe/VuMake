#include "VuBuffer.h"

//#include "VuRenderer.h"
#include "VuUtils.h"


VkResult VuBuffer::Init(VmaAllocator allocator, uint32 lenght, uint32 stride, VkBufferUsageFlags usage) {
    Allocator = allocator;
    Stride = stride;
    Lenght = lenght;

    VkBufferCreateInfo bufCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = static_cast<VkDeviceSize>(lenght * stride),
        .usage = usage
    };


    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo allocInfo;
    return vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &VulkanBuffer, &Allocation, &allocInfo);
}

void VuBuffer::Dispose() {
    vmaDestroyBuffer(Allocator, VulkanBuffer, Allocation);
}


VkResult VuBuffer::SetData(void* data, VkDeviceSize byteSize) {
    return vmaCopyMemoryToAllocation(Allocator, data, Allocation, 0, byteSize);
}


void VuBuffer::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                            VkMemoryPropertyFlags properties, VkBuffer& buffer,
                            VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(Vu::Device, &bufferInfo, nullptr, &buffer));

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(Vu::Device, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = Vu::findMemoryType(memRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(Vu::Device, &allocInfo, nullptr, &bufferMemory));

    vkBindBufferMemory(Vu::Device, buffer, bufferMemory, 0);
}


void VuBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // VkCommandBufferAllocateInfo allocInfo{};
    // allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    // allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    // allocInfo.commandPool = Vu::commandPool;
    // allocInfo.commandBufferCount = 1;
    //
    // VkCommandBuffer commandBuffer;
    // vkAllocateCommandBuffers(Vu::Device, &allocInfo, &commandBuffer);
    //
    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    //
    // vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkCommandBuffer commandBuffer = Vu::BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    Vu::EndSingleTimeCommands(commandBuffer);
    //     vkEndCommandBuffer(commandBuffer);
    //     VkSubmitInfo submitInfo{};
    //     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    //     submitInfo.commandBufferCount = 1;
    //     submitInfo.pCommandBuffers = &commandBuffer;
    //     vkQueueSubmit(Vu::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    //     vkQueueWaitIdle(Vu::graphicsQueue);
    //     vkFreeCommandBuffers(Vu::Device, Vu::commandPool, 1, &commandBuffer);
}
