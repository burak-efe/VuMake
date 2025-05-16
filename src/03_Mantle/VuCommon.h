#pragma once

#include <source_location>

#include "vulkan/vulkan_raii.hpp" // IWYU pragma: export

#define VK_RET_ON_FAIL(res)                                                                                            \
  do {                                                                                                                 \
    if ((res) != vk::Result::eSuccess) return std::unexpected {(res)};                                                 \
  } while (0)

template <typename T, typename E>
void
throw_if_unexpected(const std::expected<T, E>& exp, std::source_location loc = std::source_location::current()) {
  if (!exp) { throw std::runtime_error(std::format("Unexpected error at {}:{}", loc.file_name(), loc.line())); }
}

template <typename T, typename E>
T&&
move_or_throw(std::expected<T, E>&& exp, const char* throwMessage = "unexpected error") {
  if (!exp) {
    throw std::runtime_error(throwMessage); // or throw your own exception type
  }
  return std::move(*exp); // move the contained value out
}

namespace Vu {
// void VkCheck(VkResult res, std::source_location location = std::source_location::current());
} // namespace Vu
