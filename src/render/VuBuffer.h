#pragma once

#include "Common.h"
#include "VuCtx.h"


namespace Vu {
    struct VuBufferCreateInfo {
        VkDeviceSize lenght = 1;
        VkDeviceSize strideInBytes = 4;
        VkBufferUsageFlags vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                                  | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    };


    struct VuBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
        VmaAllocationInfo allocationInfo;
        VkDeviceSize lenght;
        VkDeviceSize stride;
        void* mapPtr;


        void init(VuBufferCreateInfo allocInfo) {

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

        void Map() {
            vmaMapMemory(ctx::vma, allocation, &mapPtr);
        }

        void Unmap() {
            vmaUnmapMemory(ctx::vma, allocation);
        }

        void uninit() {
            vmaDestroyBuffer(ctx::vma, buffer, allocation) ;
        }

        VkDeviceAddress getDeviceAddress() const {
            VkBufferDeviceAddressInfo deviceAdressInfo{};
            deviceAdressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            deviceAdressInfo.buffer = buffer;

            VkDeviceAddress address = vkGetBufferDeviceAddressKHR(ctx::device, &deviceAdressInfo);
            return address;
        }

        VkResult SetData(void* data, VkDeviceSize byteSize) {
            return vmaCopyMemoryToAllocation(ctx::vma, data, allocation, 0, byteSize);
        }

        VkResult setDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize) {
            return vmaCopyMemoryToAllocation(ctx::vma, data, allocation, offset, byteSize);
        }

        VkDeviceSize GetSizeInBytes() {
            return lenght * stride;
        }

        std::span<uint8> getSpan(VkDeviceSize start, VkDeviceSize bytelenght) {
            uint8* ptr = static_cast<uint8 *>(mapPtr);
            ptr += start;
            std::span span(ptr, bytelenght);
            return span;
        }

        static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
            VkCommandBuffer commandBuffer = ctx::BeginSingleTimeCommands();

            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

            ctx::EndSingleTimeCommands(commandBuffer);
        }

        static VkDeviceSize AlignedSize(VkDeviceSize value, VkDeviceSize alignment) {
            return (value + alignment - 1) & ~(alignment - 1);
        }




    };
}
