#include "Common.h"

#include <stacktrace>
#include <vulkan/vk_enum_string_helper.h>

namespace Vu {

    void VkCheck(VkResult res) {
        if (res != VK_SUCCESS) {
            auto st = std::stacktrace::current();
            auto msg = std::format("[ERROR] VkResult is {0} at {1} line {2}",
                                   string_VkResult(res),
                                   st[1].source_file(), st[1].source_line());

            std::cerr << msg << std::endl;
            throw std::runtime_error(msg.c_str());
        }
    }
}
