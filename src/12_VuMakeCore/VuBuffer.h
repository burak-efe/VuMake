#pragma once

#include "10_Core/VuCommon.h"
#include "VuTypes.h"

namespace Vu
{
    struct VuBufferCreateInfo
    {
        VuName             name          = "VuBuffer";
        VkDeviceSize       length        = 1;
        VkDeviceSize       strideInBytes = 1;
        VkBufferUsageFlags vkUsageFlags  = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
        VmaMemoryUsage           vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO;
        VmaAllocationCreateFlags vmaCreateFlags =
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
    };

    struct VuBuffer
    {
        VkDevice           device         = VK_NULL_HANDLE;
        VmaAllocator       vma            = VK_NULL_HANDLE;
        //VuBufferCreateInfo lastCreateInfo = {};

        VuName            name           = "VuBuffer";
        VkBuffer          buffer         = VK_NULL_HANDLE;
        VmaAllocation     allocation     = VK_NULL_HANDLE;
        VmaAllocationInfo allocationInfo = {};
        VkDeviceSize      length         = 0;
        VkDeviceSize      stride         = 0;
        void*             mapPtr         = VK_NULL_HANDLE;
        u32orNull         bindlessIndex  = 0;


        void init(VkDevice device, VmaAllocator allocator, const VuBufferCreateInfo& info);

        void uninit();

        void map();

        void unmap();

        [[nodiscard]] VkDeviceAddress getDeviceAddress() const;

        VkResult setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offset = 0) const;

        [[nodiscard]] VkDeviceSize getSizeInBytes() const;

        [[nodiscard]] std::span<byte> getMappedSpan(VkDeviceSize start, VkDeviceSize bytelenght) const;

        static VkDeviceSize alignedSize(VkDeviceSize value, VkDeviceSize alignment);
    };

    //static_assert(std::is_default_constructible_v<VuBuffer>);
}
