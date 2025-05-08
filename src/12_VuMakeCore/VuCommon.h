#pragma once

#include <source_location>

#include "vulkan/vulkan_raii.hpp" // IWYU pragma: export

#define RET_ON_FAIL(res)                 \
do {                                     \
if ((res) != vk::Result::eSuccess)       \
return std::unexpected{(res)};           \
} while (0)

// #define THROW_ON_FAIL(res)               \
// do {                                     \
// if ((res) != vk::Result::eSuccess)       \
// throw res;                               \
// } while (0)


namespace Vu
{

void VkCheck(VkResult res, std::source_location location = std::source_location::current());


} // Vu
