#pragma once

#include "Common.h"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class VuBuffer {
public:
    VkBuffer VulkanBuffer;
    VmaAllocator Allocator;
    VmaAllocation Allocation;
    uint32 Lenght;
    uint32 Stride;


    VuBuffer();

    VkResult Init(VmaAllocator allocator, uint32 lenght, uint32 stride, VkBufferUsageFlags usage);

    void Dispose();

    VkResult SetData(void* data, VkDeviceSize byteSize);


    //TEMP
    //static uint32 findMemoryType(uint32 typeFilter, VkMemoryPropertyFlags properties);

    static void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                             VkDeviceMemory& bufferMemory);

    //
    // void Map();
    //
    // void UnMap();
};
