#include "VuCommon.h"

#include <format>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
