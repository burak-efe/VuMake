#include "VuBuffer.h"

#include "VuUtils.h"


VuBuffer::VuBuffer(VmaAllocator allocator, uint32 lenght, uint32 stride, const VkBufferUsageFlags usage,
                   VmaAllocationCreateFlags flags) {

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
    allocCreateInfo.flags = flags;

    VK_CHECK(vmaCreateBuffer(allocator, &bufCreateInfo, &allocCreateInfo, &Buffer, &Allocation, &AllocationInfo));
}

void VuBuffer::Dispose() {
    vmaDestroyBuffer(Allocator, Buffer, Allocation);
}

VkResult VuBuffer::SetData(void* data, VkDeviceSize byteSize) {
    return vmaCopyMemoryToAllocation(Allocator, data, Allocation, 0, byteSize);
}

VkResult VuBuffer::SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize) {
    return vmaCopyMemoryToAllocation(Allocator, data, Allocation, offset, byteSize);
}

void VuBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = Vu::BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    Vu::EndSingleTimeCommands(commandBuffer);
}

VkDeviceSize VuBuffer::GetDeviceSize() {
    return Lenght * Stride;
}
