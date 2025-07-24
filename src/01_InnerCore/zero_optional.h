#pragma once

#include <optional> // for std::nullopt_t
#include <stdexcept>
#include <type_traits>
#include <utility>

// Custom Optional to express 0 value as null, the advantage over the regular std::optional is this version have zero
// overhead
template <typename T> class zero_optional {
  static_assert(std::is_integral_v<T>, "nullable<T> only supports integral types");

  T value_ = 0;

public:
  // === Constructors ===
  constexpr zero_optional() noexcept = default;
  constexpr zero_optional(std::nullopt_t) noexcept :
      value_(0) {}
  constexpr zero_optional(const zero_optional&) noexcept = default;
  constexpr zero_optional(zero_optional&&) noexcept      = default;

  constexpr zero_optional&
  operator=(const zero_optional&) noexcept = default;
  constexpr zero_optional&
  operator=(zero_optional&&) noexcept = default;

  constexpr zero_optional(T value) noexcept :
      value_(value) {}
  constexpr zero_optional&
  operator=(T value) noexcept {
    value_ = value;
    return *this;
  }

  constexpr zero_optional&
  operator=(std::nullopt_t) noexcept {
    value_ = 0;
    return *this;
  }

  // === Observers ===
  constexpr bool
  has_value() const noexcept {
    return value_ != 0;
  }
  constexpr explicit
  operator bool() const noexcept {
    return has_value();
  }

  constexpr T
  value_or_THROW() const {
    if (!has_value()) throw std::bad_optional_access();
    return value_;
  }

  template <typename U>
  constexpr T
  value_or(U&& default_value) const {
    return has_value() ? value_ : static_cast<T>(std::forward<U>(default_value));
  }

  // === Modifiers ===
  constexpr void
  reset() noexcept {
    value_ = 0;
  }

  // === Dereference operators ===
  constexpr const T&
  operator*() const noexcept {
    return value_;
  }
  constexpr T&
  operator*() noexcept {
    return value_;
  }

  constexpr const T*
  operator->() const noexcept {
    return &value_;
  }
  constexpr T*
  operator->() noexcept {
    return &value_;
  }

  // === Comparisons ===
  friend constexpr bool
  operator==(const zero_optional& lhs, const zero_optional& rhs) noexcept {
    return lhs.value_ == rhs.value_;
  }

  friend constexpr bool
  operator!=(const zero_optional& lhs, const zero_optional& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend constexpr bool
  operator==(const zero_optional& lhs, std::nullopt_t) noexcept {
    return !lhs.has_value();
  }

  friend constexpr bool
  operator==(std::nullopt_t, const zero_optional& rhs) noexcept {
    return !rhs.has_value();
  }

  friend constexpr bool
  operator!=(const zero_optional& lhs, std::nullopt_t) noexcept {
    return lhs.has_value();
  }

  friend constexpr bool
  operator!=(std::nullopt_t, const zero_optional& rhs) noexcept {
    return rhs.has_value();
  }
};
