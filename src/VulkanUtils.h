#pragma once

#include "vulkan/vk_enum_string_helper.h"
#include <fstream>



#define VK_CHECK(f)																				\
{																										\
VkResult res = (f);																					\
if (res != VK_SUCCESS)																				\
{																									\
std::cout << "Fatal : VkResult is \"" << string_VkResult(res) << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
assert(res == VK_SUCCESS);																		\
}																									\
}





static std::vector<char> readFile(const std::string &filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
