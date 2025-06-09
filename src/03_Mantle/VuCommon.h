#pragma once

#include <iostream>
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
move_or_THROW(std::expected<T, E>&& exp, const char* throwMessage = "unexpected error", std::source_location loc = std::source_location::current()) {
  if (!exp) {
    throw std::runtime_error(std::format("{},\n at {}, line {},\n function {}\n",throwMessage,loc.file_name(),loc.line(),loc.function_name())); // or throw your own exception type
  }
  return std::move(*exp); // move the contained value out
}

namespace Vu {
} // namespace Vu
