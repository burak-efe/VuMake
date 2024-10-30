#pragma once

#include "Common.h"
#include "VuCtx.h"

namespace Vu {
    struct VuBufferAllocInfo {
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


        void Alloc(VuBufferAllocInfo allocInfo);

        void Map();

        void Unmap();

        void Dispose();

        VkResult SetData(void* data, VkDeviceSize byteSize);

        VkResult SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize);

        VkDeviceSize GetSizeInBytes();

        static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        static VkDeviceSize AlignedSize(VkDeviceSize value, VkDeviceSize alignment);

        static VkDeviceAddress GetDeviceAddress(VkDevice device, VkBuffer buffer);

    };
}
