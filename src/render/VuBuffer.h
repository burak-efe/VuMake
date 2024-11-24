#pragma once

#include "Common.h"
//#include "VuCtx.h"

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


        void init(VuBufferCreateInfo allocInfo);

        void Map();

        void Unmap();

        void uninit();

        VkDeviceAddress getDeviceAddress() const;

        VkResult SetData(void* data, VkDeviceSize byteSize);

        VkResult setDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize);

        VkDeviceSize GetSizeInBytes();

        std::span<uint8> getSpan(VkDeviceSize start, VkDeviceSize bytelenght) {
            uint8* ptr = static_cast<uint8 *>(mapPtr);
            ptr += start;
            std::span span(ptr, bytelenght);
            return span;
        }

        static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        static VkDeviceSize AlignedSize(VkDeviceSize value, VkDeviceSize alignment);

        //static VkDeviceAddress GetDeviceAddress(VkDevice device, VkBuffer buffer);


    };
}
