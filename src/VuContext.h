#pragma once
#include <vk_mem_alloc.h>

namespace VuContext {
    inline VmaAllocator VmaAllocator = VK_NULL_HANDLE;
    inline VkInstance Instance = VK_NULL_HANDLE;
    inline VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
    inline VkDevice Device = VK_NULL_HANDLE;
}
