#pragma once

#include <utility>

#include "VuUtils.h"
#include "VuCommon.h"

namespace Vu
{

struct VuDeviceAllocator
{
    std::shared_ptr<vk::raii::Device> device;
    vk::PhysicalDeviceMemoryProperties  phyDeviceMemProps;

    VuDeviceAllocator(std::shared_ptr<vk::raii::Device>       device,
                      const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties) :
        device{std::move(device)},
        phyDeviceMemProps{physicalDeviceMemoryProperties}
    {
    }

    std::expected<vk::raii::DeviceMemory, vk::Result>
    allocateMemory(const vk::DeviceSize          sizeInByte,
                   const vk::MemoryPropertyFlags memPropFlags,
                   const vk::MemoryRequirements& requirements)
    {


        auto memTypeIndex = Utils::findMemoryTypeIndex(phyDeviceMemProps,
                                                       requirements.memoryTypeBits,
                                                       memPropFlags);

        if (!memTypeIndex)
        {
            return std::unexpected{memTypeIndex.error()};
        }

        vk::MemoryAllocateInfo allocInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .pNext = VK_NULL_HANDLE,
                .allocationSize = sizeInByte,
                .memoryTypeIndex = memTypeIndex.value()
        };

        vk::DeviceMemory memory;

        std::expected<vk::raii::DeviceMemory, vk::Result> memoryExpected = device->allocateMemory(allocInfo);

        return memoryExpected;

        if (allocResult != VK_SUCCESS)
        {
            return std::unexpected{allocResult};
        }

        return memory;

    }

    void freeMemory();

};

} // Vu
