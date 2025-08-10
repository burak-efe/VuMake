#pragma once

#include <expected>
#include <memory>

#include "Common.h"
#include "vulkan/vulkan.h"

#define RETURN_UNEXPECTED_ON_FAIL(res)                                                                                 \
  do {                                                                                                                 \
    if (res != VK_SUCCESS) return std::unexpected {res};                                                               \
  } while (0)

template <typename T, typename E>
void
THROW_if_unexpected(const std::expected<T, E>& exp, std::source_location loc = std::source_location::current()) {
  if (!exp) { throw std::runtime_error(std::format("Unexpected error at {}:{}", loc.file_name(), loc.line())); }
}

template <typename T, typename E>
T&&
move_or_THROW(std::expected<T, E>&& exp,
              const char*           throwMessage = "unexpected error",
              std::source_location  loc          = std::source_location::current()) {
  if (!exp) {
    throw std::runtime_error(std::format("{},\n at {}, line {},\n function {}\n",
                                         throwMessage,
                                         loc.file_name(),
                                         loc.line(),
                                         loc.function_name())); // or throw your own exception type
  }
  return std::move(*exp); // move the contained value out
}

template <typename T, typename E>
T&&
move_or_THROW(std::expected<T, E>& exp,
              const char*          throwMessage = "unexpected error",
              std::source_location loc          = std::source_location::current()) {
  if (!exp) {
    throw std::runtime_error(std::format(
        "{},\n at {}, line {},\n function {}\n", throwMessage, loc.file_name(), loc.line(), loc.function_name()));
  }
  return std::move(*exp);
}

void
THROW_if_fail(VkResult res);

constexpr VkDeviceSize
MakeVkOffset(uint64_t val) {
  return val;
}

#define NO_ALLOC_CALLBACK nullptr

// #define SETUP_VU_OBJECT(Type)                                                                                          \
//   Type(std::nullptr_t) {}                                                                                              \
//   Type(const Type&)            = delete;                                                                               \
//   Type& operator=(const Type&) = delete;                                                                               \
//   //Type(Type&&)                 = default;                                                                              \
//   //Type& operator=(Type&&)      = default;

#define EXPAND_ARGS(...) __VA_ARGS__

#define SETUP_EXPECTED_WRAPPER(CLASS_NAME, PARAMS, ARGS)                                                               \
  inline static std::expected<CLASS_NAME, VkResult> make /**/                                                          \
      PARAMS noexcept {                                                                                                \
    try {                                                                                                              \
      CLASS_NAME outObject {EXPAND_ARGS ARGS};                                                                         \
      return std::move(outObject);                                                                                     \
    } catch (VkResult res) { return std::unexpected(res); } catch (...) {                                              \
      return std::unexpected(VK_ERROR_UNKNOWN);                                                                        \
    }                                                                                                                  \
  }

// static_assert(std::is_default_constructible_v<VuImage> == false);
// static_assert(std::is_copy_assignable_v<VuImage> == false);
// static_assert(std::is_copy_constructible_v<VuImage> == false);
//
// static_assert(std::is_move_constructible_v<VuImage> == true);
// static_assert(std::is_move_assignable_v<VuImage> == true);
// static_assert(std::is_destructible_v<VuImage> == true);
//
// static_assert(std::is_nothrow_destructible_v<VuImage> == true);
// static_assert(std::is_nothrow_move_assignable_v<VuImage> == true);
// static_assert(std::is_nothrow_move_constructible_v<VuImage> == true);
//
// static_assert(std::is_constructible_v<VuImage, std::nullptr_t>, "must be constructible from nullptr");
