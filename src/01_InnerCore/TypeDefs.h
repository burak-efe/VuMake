#pragma once
#include <cstdint>
#include <climits>
#include <stdexcept>

using byte = std::byte;

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

// using u8orNull  = uint8_t;
// using u16orNull = uint16_t;
// using u32orNull = uint32_t;
// using u64orNull = uint64_t;
//
// using i8orNull  = int8_t;
// using i16orNull = int16_t;
// using i32orNull = int32_t;
// using i64orNull = int64_t;

template <typename T> using PtrOrNull = T*;

// template <typename T> using vector = std::pmr::vector<T>;
// using string = std::pmr::string;

consteval std::uint8_t
operator""_u8(unsigned long long value) {
  if (value > UCHAR_MAX) { throw std::out_of_range("Value exceeds uint8_t range"); }
  return static_cast<std::uint8_t>(value);
}

consteval std::uint16_t
operator""_u16(unsigned long long value) {
  if (value > UINT16_MAX) { throw std::out_of_range("Value exceeds uint16_t range"); }
  return static_cast<std::uint16_t>(value);
}
