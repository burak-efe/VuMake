#pragma once

#include "Common.h"


class VuBuffer {
public:
    VkBuffer VulkanBuffer;
    VmaAllocator Allocator;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;
    uint32 Lenght;
    uint32 Stride;

    VuBuffer() = default;

    VuBuffer(VmaAllocator allocator, uint32 lenght, uint32 stride, VkBufferUsageFlags usage,
             VmaAllocationCreateFlags flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    //VkResult Init(VmaAllocator allocator, uint32 lenght, uint32 stride, VkBufferUsageFlags usage);

    void Dispose();

    VkResult SetData(void* data, VkDeviceSize byteSize);

    static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                             VkDeviceMemory& bufferMemory);

};
