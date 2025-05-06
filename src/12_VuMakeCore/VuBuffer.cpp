#include "VuBuffer.h"

#include "08_LangUtils/VuLogger.h"

namespace Vu
{
    void VuBuffer::init(VkDevice device, VmaAllocator allocator, const VuBufferCreateInfo& info)
    {
        vma          = allocator;
        this->device = device;
        //lastCreateInfo = info;
        stride     = info.strideInBytes;
        length     = info.length;
        this->name = info.name;


        VkBufferCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = (length * stride),
            .usage = info.vkUsageFlags | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
        };

        VmaAllocationCreateInfo allocCreateInfo = {};

        allocCreateInfo.usage = info.vmaMemoryUsage;
        allocCreateInfo.flags = info.vmaCreateFlags;

        VkCheck(vmaCreateBuffer(vma, &createInfo, &allocCreateInfo, &buffer, &allocation, &allocationInfo));
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
        return length * stride;
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
