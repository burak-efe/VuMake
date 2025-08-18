#pragma once

#include <span>

namespace Vu {
template <typename T_From, typename T_To>
std::span<T_To>
rpCastSpan(std::span<T_From> source) {
  static_assert(sizeof(T_From) == sizeof(T_To), "T_From and T_To must be the same size for reinterpret casting.");
  return std::span<T_To>(reinterpret_cast<T_To*>(source.data()), source.size());
}
} // namespace Vu
