#include "VuBuffer.h"

#include "VuDeviceAllocator.h"
#include "VuUtils.h"
#include "08_LangUtils/VuLogger.h"  // for Logger

namespace Vu
{
void VuBuffer::init(std::shared_ptr<vk::raii::Device> device,
                    const VuBufferCreateInfo&         createInfo,
                    VuDeviceAllocator&                allocator)
{
    this->device = device;
    sizeInBytes  = createInfo.sizeInBytes;
    name         = createInfo.name;


    vk::BufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.size  = sizeInBytes;
    bufferCreateInfo.usage = vk::BufferUsageFlagBits::eShaderDeviceAddress;


    vk::raii::Buffer t = device->createBuffer(bufferCreateInfo);


    std::shared_ptr<vk::raii::PhysicalDevice> dev;

    // create a vk::raii::Device, given a vk::raii::PhysicalDevice physicalDevice and a vk::DeviceCreateInfo deviceCreateInfo
    // when VULKAN_HPP_NO_EXCEPTIONS is defined and your using at least C++23
    auto deviceExpected = dev->createDevice( {});
    if ( deviceExpected.has_value() )
    {
        device = std::move( *deviceExpected );
    }

#if VULKAN_HPP_EXPECTED
    hello
#endif

    vk::DeviceBufferMemoryRequirements bufferMemReq{.pCreateInfo = bufferCreateInfo};

    vk::MemoryRequirements2 memory_requirements = device->getBufferMemoryRequirements(bufferMemReq);

    allocator.allocateMemory(sizeInBytes, bufferCreateInfo.vkMemoryPropertyFlags, memory_requirements);
    device->bindBufferMemory2(buffer, deviceMemory, bufferCreateInfo.vkMemoryAllocateFlags);

    Logger::Trace("{} init", name.c_str());
}

void VuBuffer::uninit()
{
    if (mapPtr != nullptr)
    {
        unmap();
    }
    vmaDestroyBuffer(vma, buffer, allocation);
    Logger::Trace("{} uninit", name.c_str());
}

void VuBuffer::map()
{
    vmaMapMemory(vma, allocation, &mapPtr);
}

void VuBuffer::unmap()
{
    vmaUnmapMemory(vma, allocation);
    mapPtr = nullptr;
}

VkDeviceAddress VuBuffer::getDeviceAddress() const
{
    VkBufferDeviceAddressInfo deviceAdressInfo{};
    deviceAdressInfo.sType  = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    deviceAdressInfo.buffer = buffer;

    VkDeviceAddress address = vkGetBufferDeviceAddress(device, &deviceAdressInfo);
    return address;
}

VkResult VuBuffer::setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offset) const
{
    return vmaCopyMemoryToAllocation(vma, data, allocation, offset, byteSize);
}

VkDeviceSize VuBuffer::getSizeInBytes() const
{
    return sizeInBytes;
}

std::span<byte> VuBuffer::getMappedSpan(VkDeviceSize start, VkDeviceSize bytelenght) const
{
    auto* ptr = static_cast<byte*>(mapPtr);
    ptr += start;
    return std::span(ptr, bytelenght);
}


VkDeviceSize VuBuffer::alignedSize(VkDeviceSize value, VkDeviceSize alignment)
{
    return (value + alignment - 1) & ~(alignment - 1);
}
}
