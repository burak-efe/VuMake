#include "VuBuffer.h"

#include "VuCtx.h"
#include "VuDevice.h"

namespace Vu{
    void VuBuffer::init(const VuBufferCreateInfo& info) {

        createInfo = info;
        stride = info.strideInBytes;
        length = info.length;

        VkBufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = (length * stride),
            .usage = info.vkUsageFlags
        };

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = info.vmaMemoryUsage;
        allocCreateInfo.flags = info.vmaCreateFlags;

        auto vma = ctx::vuDevice->vma;
        VkCheck(vmaCreateBuffer(vma, &createInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));
    }

    void VuBuffer::uninit() {
        if (mapPtr!= nullptr) {
            unmap();
        }
        vmaDestroyBuffer(ctx::vuDevice->vma, buffer, allocation);
    }

    void VuBuffer::map() {
        vmaMapMemory(ctx::vuDevice->vma, allocation, &mapPtr);
    }

    void VuBuffer::unmap() {
        vmaUnmapMemory(ctx::vuDevice->vma, allocation);
        mapPtr = nullptr;
    }

    VkDeviceAddress VuBuffer::getDeviceAddress() const {
        VkBufferDeviceAddressInfo deviceAdressInfo{};
        deviceAdressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAdressInfo.buffer = buffer;

        VkDeviceAddress address = vkGetBufferDeviceAddress(ctx::vuDevice->device, &deviceAdressInfo);
        return address;
    }

    VkResult VuBuffer::setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offset) const
    {
        return vmaCopyMemoryToAllocation(ctx::vuDevice->vma, data, allocation, offset, byteSize);
    }

    VkDeviceSize VuBuffer::getSizeInBytes() const
    {
        return length * stride;
    }

    std::span<uint8> VuBuffer::getMappedSpan(VkDeviceSize start, VkDeviceSize bytelenght) const
    {
        auto* ptr = static_cast<uint8 *>(mapPtr);
        ptr += start;
        return std::span(ptr, bytelenght);
    }

    void VuBuffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = ctx::vuDevice->BeginSingleTimeCommands();
        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;
        copyRegion.dstOffset = 0;
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        ctx::vuDevice->EndSingleTimeCommands(commandBuffer);
    }

    VkDeviceSize VuBuffer::alignedSize(VkDeviceSize value, VkDeviceSize alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }
}
