#include "VuCommon.h"

#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION
#include "volk.h"

#define VMA_IMPLEMENTATION 1
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"

#define STB_IMAGE_IMPLEMENTATION
#include <format>
#include <stb_image.h>

#include <iostream>
#include <vulkan/vk_enum_string_helper.h>

namespace Vu
{
    void VkCheck(VkResult res, std::source_location location)
    {
        if (res != VK_SUCCESS)
        {
            auto msg = std::format("[ERROR] VkResult is {0} at {1}, line {2}, column {3}",
                                   string_VkResult(res),
                                   location.file_name(),
                                   location.line(),
                                   location.column()
                                  );

            std::cerr << msg << std::endl;
            //throw std::runtime_error(msg.c_str());
        }
    }
}
