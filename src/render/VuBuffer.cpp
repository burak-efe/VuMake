#include "VuBuffer.h"

#include "VuUtils.h"

namespace Vu {
    void VuBuffer::Alloc(VuBufferAllocInfo allocInfo) {

        stride = allocInfo.strideInBytes;
        lenght = allocInfo.lenght;

        VkBufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = static_cast<VkDeviceSize>(lenght * stride),
            .usage = allocInfo.vkUsageFlags
        };

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = allocInfo.vmaMemoryUsage;
        allocCreateInfo.flags = allocInfo.vmaCreateFlags;

        VkCheck(vmaCreateBuffer(ctx::vma, &createInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));
    }

    void VuBuffer::Map() {
        vmaMapMemory(ctx::vma, allocation, &mapPtr);
    }

    void VuBuffer::Unmap() {
        vmaUnmapMemory(ctx::vma, allocation);
    }

    void VuBuffer::Dispose() {
        vmaDestroyBuffer(ctx::vma, buffer, allocation);
    }

    VkResult VuBuffer::SetData(void* data, VkDeviceSize byteSize) {
        return vmaCopyMemoryToAllocation(ctx::vma, data, allocation, 0, byteSize);
    }

    VkResult VuBuffer::SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize) {
        return vmaCopyMemoryToAllocation(ctx::vma, data, allocation, offset, byteSize);
    }

    void VuBuffer::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = ctx::BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        ctx::EndSingleTimeCommands(commandBuffer);
    }

    VkDeviceSize VuBuffer::GetSizeInBytes() {
        return lenght * stride;
    }

    VkDeviceSize VuBuffer::AlignedSize(VkDeviceSize value, VkDeviceSize alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    VkDeviceAddress VuBuffer::GetDeviceAddress(VkDevice device, VkBuffer buffer) {
        VkBufferDeviceAddressInfo deviceAdressInfo{};
        deviceAdressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAdressInfo.buffer = buffer;
        uint64_t address = vkGetBufferDeviceAddress(device, &deviceAdressInfo);
        return address;
    }
}
