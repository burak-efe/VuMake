#pragma once

#include "Common.h"
#include "Vu.h"

struct VuBufferAllocInfo {
    VkDeviceSize lenght = 1;
    VkDeviceSize strideInBytes = 4;
    VkBufferUsageFlags vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    VmaMemoryUsage vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO;
    VmaAllocationCreateFlags vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
};


class VuBuffer {
public:
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkDeviceSize Lenght;
    VkDeviceSize Stride;
    void* mapPtr;


    void Alloc(VuBufferAllocInfo allocInfo);

    void Map() {
        vmaMapMemory(Vu::VmaAllocator, allocation, &mapPtr);
    }

    void Unmap() {
        vmaUnmapMemory(Vu::VmaAllocator, allocation);
    }


    void Dispose();

    VkResult SetData(void* data, VkDeviceSize byteSize);

    VkResult SetDataWithOffset(void* data, VkDeviceSize offset, VkDeviceSize byteSize);

    static void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkDeviceSize GetDeviceSize();


    VkDeviceSize aligned_size(VkDeviceSize value, VkDeviceSize alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    VkDeviceAddress get_device_address(VkDevice device, VkBuffer buffer) {
        VkBufferDeviceAddressInfo deviceAdressInfo{};
        deviceAdressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAdressInfo.buffer = buffer;
        uint64_t address = vkGetBufferDeviceAddress(device, &deviceAdressInfo);
        return address;
    }

};
