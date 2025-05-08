#pragma once

#include <span>                     // for span

#include "08_LangUtils/TypeDefs.h"  // for byte, u32orNull
#include "VuCommon.h"
#include "VuTypes.h"                // for VuName

namespace Vu
{
struct VuDeviceAllocator;
}

namespace Vu
{
struct VuBufferCreateInfo
{
    VuName               name         = "VuBuffer";
    vk::DeviceSize       sizeInBytes  = 1;
    vk::BufferUsageFlags vkUsageFlags = vk::BufferUsageFlagBits::eStorageBuffer
                                        | vk::BufferUsageFlagBits::eShaderDeviceAddress;
    vk::MemoryAllocateFlags vkMemoryAllocateFlags = vk::MemoryAllocateFlagBits::eDeviceAddress;
    vk::MemoryPropertyFlags vkMemoryPropertyFlags = vk::MemoryPropertyFlagBits::eDeviceLocal
                                                    | vk::MemoryPropertyFlagBits::eHostVisible
                                                    | vk::MemoryPropertyFlagBits::eHostCoherent;
};


struct VuBuffer
{
    std::shared_ptr<vk::raii::Device> device       = VK_NULL_HANDLE;
    vk::raii::DeviceMemory            deviceMemory = VK_NULL_HANDLE;
    vk::raii::Buffer                  buffer       = VK_NULL_HANDLE;

    void* mapPtr = VK_NULL_HANDLE;

    vk::DeviceSize sizeInBytes   = 0;
    u32orNull      bindlessIndex = 0;
    VuName         name          = "VuBuffer";


    void init(std::shared_ptr<vk::raii::Device> device, const VuBufferCreateInfo& createInfo,
              VuDeviceAllocator&                allocator);

    void uninit();

    void map();

    void unmap();

    [[nodiscard]] vk::DeviceAddress getDeviceAddress() const;

    vk::Result setData(const void* data, vk::DeviceSize byteSize, vk::DeviceSize offset = 0) const;

    [[nodiscard]] vk::DeviceSize getSizeInBytes() const;

    [[nodiscard]] std::span<byte> getMappedSpan(vk::DeviceSize start, vk::DeviceSize bytelenght) const;

    static vk::DeviceSize alignedSize(vk::DeviceSize value, vk::DeviceSize alignment);
};
}
