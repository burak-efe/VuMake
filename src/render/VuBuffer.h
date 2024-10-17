#pragma once

#include "Common.h"


class VuBuffer {
public:
    VkBuffer Buffer;
    //VmaAllocator Allocator;
    VmaAllocation Allocation;
    VmaAllocationInfo AllocationInfo;
    VkDeviceSize Lenght;
    VkDeviceSize Stride;
    //void* mapPtr;

    //VuBuffer() = default;

    void Alloc(uint32 lenght, uint32 stride,
             VkBufferUsageFlags vkUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
             VmaAllocationCreateFlags vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);

    //VkResult Init(VmaAllocator allocator, uint32 lenght, uint32 stride, VkBufferUsageFlags usage);

    void Dispose();

    VkResult SetData(void* data, VkDeviceSize byteSize);

    VkResult SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize);

    static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDeviceSize GetDeviceSize();

    // static void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
    //                          VkDeviceMemory& bufferMemory);

};
