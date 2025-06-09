#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>

namespace Vu {

template <std::size_t TN_Capacity, typename T_Counter> struct FixedString {
  static_assert(std::is_integral_v<T_Counter>, "T_counter must be an integral type");
  static_assert(TN_Capacity - 1 <= static_cast<std::size_t>(std::numeric_limits<T_Counter>::max()),
                "T_counter type is too small for the specified Capacity");

private:
  std::array<char, TN_Capacity> buffer;
  T_Counter                     length = 0;

public:
  constexpr FixedString() : buffer(), length(0) {}

  constexpr FixedString(const char* str) { assign(str); }

  constexpr FixedString(const char* str, std::size_t length) { assign(str, length); }

  constexpr void
  assign(const char* str) {
    std::size_t len = std::char_traits<char>::length(str);
    if (len >= TN_Capacity) throw std::out_of_range("FixedString: input string too long");

    std::copy_n(str, len, buffer.begin());
    buffer[len] = '\0';
    length      = len;
  }

  constexpr void
  assign(const char* str, std::size_t len) {
    if (len >= TN_Capacity) throw std::out_of_range("FixedString: input string too long");

    std::copy_n(str, len, buffer.begin());
    buffer[len] = '\0';
    length      = len;
  }

  constexpr const char*
  c_str() const {
    return buffer.data();
  }

  constexpr std::string_view
  view() const {
    return std::string_view(buffer, length);
  }

  constexpr std::size_t
  size() const {
    return length;
  }

  static constexpr std::size_t
  capacity() {
    return TN_Capacity;
  }

  constexpr bool
  empty() const {
    return length == 0;
  }

  constexpr char&
  operator[](std::size_t i) {
    return buffer[i];
  }

  constexpr const char&
  operator[](std::size_t i) const {
    if (i >= length) { throw std::out_of_range("FixedString index out of bounds"); }
    return buffer[i];
  }

  constexpr bool
  operator==(const FixedString& other) const {
    return length == other.length && std::equal(buffer, buffer + length, other.buffer);
  }

  constexpr bool
  operator!=(const FixedString& other) const {
    return !(*this == other);
  }
};

using FixedString64 = FixedString<63, uint8_t>;
static_assert(sizeof(FixedString64) == 64, "Expected 64 bytes");

using FixedString128 = FixedString<127, uint8_t>;
static_assert(sizeof(FixedString128) == 128, "Expected 64 bytes");

using FixedString512 = FixedString<510, uint16_t>;
static_assert(sizeof(FixedString512) == 512, "Expected 64 bytes");
} // namespace Vu
