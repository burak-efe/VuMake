#pragma once

namespace Vu::Math {
struct Float2;
struct Float3;

struct Float4 {
  float x;
  float y;
  float z;
  float w;

  // Constructors
  Float4();

  Float4(float x, float y, float z, float w);

  Float4(const Float3& xyz, float w);

  Float4(const Float2& xy, const Float2& zw);

  // Basic operators
  Float4&
  operator+=(const Float4& rhs);

  Float4&
  operator-=(const Float4& rhs);

  Float4&
  operator*=(float scalar);

  Float4&
  operator/=(float scalar);

  // Conversion to Float3/Float2
  Float3
  xyz() const;

  Float2
  xy() const;
};

// Non-member operators for Float4
Float4
operator+(Float4 lhs, const Float4& rhs);

Float4
operator-(Float4 lhs, const Float4& rhs);

Float4
operator*(Float4 vec, float scalar);

Float4
operator*(float scalar, Float4 vec);

Float4
operator/(Float4 vec, float scalar);

Float4
operator-(const Float4& vec);

// Float4 utility functions
float
length(const Float4& vec);

float
lengthSquared(const Float4& vec);

Float4
normalize(const Float4& vec);

float
dot(const Float4& a, const Float4& b);

Float4
lerp(const Float4& a, const Float4& b, float t);

} // namespace Vu::Math
