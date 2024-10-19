#pragma once

#include "Common.h"


class VuBuffer {
public:
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkDeviceSize Lenght;
    VkDeviceSize Stride;


    void Alloc(uint32 lenght, uint32 stride,
             VkBufferUsageFlags vkUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
             VmaAllocationCreateFlags vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT);


    void Dispose();

    VkResult SetData(void* data, VkDeviceSize byteSize);

    VkResult SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize);

    static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDeviceSize GetDeviceSize();

};
