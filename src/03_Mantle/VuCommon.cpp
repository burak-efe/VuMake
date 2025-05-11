#include "VuCommon.h"

#include <format>
#include <iostream>

#include "vulkan/vk_enum_string_helper.h"
#include "vulkan/vulkan_raii.hpp"


namespace Vu
{

// void VkCheck(vk::Result res, std::source_location location)
// {
//     if (res != vk::Result::eSuccess)
//     {
//         auto msg = std::format("[ERROR] VkResult is {0} at {1}, line {2}, column {3}",
//                                string_VkResult(res),
//                                location.file_name(),
//                                location.line(),
//                                location.column()
//                 );
//
//         std::cerr << msg << std::endl;
//         //throw std::runtime_error(msg.c_str());
//     }
// }


} // Vu
