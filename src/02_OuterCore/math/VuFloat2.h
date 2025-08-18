#pragma once

namespace Vu::Math {
struct Float2 {
  float x;
  float y;

  // Constructors
  Float2();

  Float2(float x, float y);

  // Basic operators
  Float2&
  operator+=(const Float2& rhs);

  Float2&
  operator-=(const Float2& rhs);

  Float2&
  operator*=(float scalar);

  Float2&
  operator/=(float scalar);
};

// Non-member operators for Float2
Float2
operator+(Float2 lhs, const Float2& rhs);

Float2
operator-(Float2 lhs, const Float2& rhs);

Float2
operator*(Float2 vec, const float scalar);

Float2
operator*(const float scalar, Float2 vec);

Float2
operator/(Float2 vec, const float scalar);

Float2
operator-(const Float2& vec);

float
length(const Float2& vec);

float
lengthSquared(const Float2& vec);

Float2
normalize(const Float2& vec);

float
dot(const Float2& a, const Float2& b);

Float2
lerp(const Float2& a, const Float2& b, float t);

} // namespace Vu::Math
