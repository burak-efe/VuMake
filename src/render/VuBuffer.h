#pragma once

#include "vk_mem_alloc.h"

#include "Common.h"

namespace Vu {

    struct VuBufferCreateInfo {
        VkDeviceSize             length         = 1;
        VkDeviceSize             strideInBytes  = 4;
        VkBufferUsageFlags       vkUsageFlags   = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VmaMemoryUsage           vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    };

    struct VuBuffer {
        VuBufferCreateInfo createInfo;
        VkBuffer           buffer;
        VmaAllocation      allocation;
        VmaAllocationInfo  allocationInfo;
        VkDeviceSize       length;
        VkDeviceSize       stride;
        void*              mapPtr;


        void init(const VuBufferCreateInfo& info);

        void uninit();

        void map();

        void unmap();

        VkDeviceAddress getDeviceAddress() const;

        VkResult setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offset = 0) const;

        VkDeviceSize getSizeInBytes() const;

        std::span<uint8> getMappedSpan(VkDeviceSize start, VkDeviceSize bytelenght) const;

        static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

        static VkDeviceSize alignedSize(VkDeviceSize value, VkDeviceSize alignment);
    };
}
