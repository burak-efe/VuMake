#include "VuBuffer.h"

#include "VuUtils.h"


void VuBuffer::Alloc(uint32 lenght, uint32 stride,
                     VkBufferUsageFlags vkUsageFlags,
                     VmaMemoryUsage vmaMemoryUsage,
                     VmaAllocationCreateFlags vmaCreateFlags) {

    Stride = stride;
    Lenght = lenght;

    VkBufferCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = static_cast<VkDeviceSize>(lenght * stride),
        .usage = vkUsageFlags
    };

    VmaAllocationCreateInfo allocCreateInfo = {};
    allocCreateInfo.usage = vmaMemoryUsage;
    allocCreateInfo.flags = vmaCreateFlags;

    VK_CHECK(vmaCreateBuffer(Vu::VmaAllocator, &createInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));
}

void VuBuffer::Dispose() {
    vmaDestroyBuffer(Vu::VmaAllocator, buffer, allocation);
}

VkResult VuBuffer::SetData(void* data, VkDeviceSize byteSize) {
    return vmaCopyMemoryToAllocation(Vu::VmaAllocator, data, allocation, 0, byteSize);
}

VkResult VuBuffer::SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize) {
    return vmaCopyMemoryToAllocation(Vu::VmaAllocator, data, allocation, offset, byteSize);
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
