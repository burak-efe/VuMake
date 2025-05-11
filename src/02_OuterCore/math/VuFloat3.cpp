#include "VuFloat3.h"
#include "VuFloat.h"
#include "VuFloat2.h"
#include <cmath>
Vu::Math::Float3::Float3() : x(0.0f), y(0.0f), z(0.0f) {}

Vu::Math::Float3::Float3(float x, float y, float z) : x(x), y(y), z(z) {}

Vu::Math::Float3::Float3(const Float2& xy, float z) : x(xy.x), y(xy.y), z(z) {}

Vu::Math::Float3&
Vu::Math::Float3::operator+=(const Float3& rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  return *this;
}

Vu::Math::Float3&
Vu::Math::Float3::operator-=(const Float3& rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  return *this;
}

Vu::Math::Float3&
Vu::Math::Float3::operator*=(float scalar) {
  x *= scalar;
  y *= scalar;
  z *= scalar;
  return *this;
}

Vu::Math::Float3&
Vu::Math::Float3::operator/=(float scalar) {
  float invScalar = 1.0f / scalar;
  x *= invScalar;
  y *= invScalar;
  z *= invScalar;
  return *this;
}

Vu::Math::Float2
Vu::Math::Float3::xy() const {
  return Float2(x, y);
}

Vu::Math::Float3
Vu::Math::operator+(Float3 lhs, const Float3& rhs) {
  lhs += rhs;
  return lhs;
}

Vu::Math::Float3
Vu::Math::operator-(Float3 lhs, const Float3& rhs) {
  lhs -= rhs;
  return lhs;
}

Vu::Math::Float3
Vu::Math::operator*(Float3 vec, float scalar) {
  vec *= scalar;
  return vec;
}

Vu::Math::Float3
Vu::Math::operator*(float scalar, Float3 vec) {
  vec *= scalar;
  return vec;
}

Vu::Math::Float3
Vu::Math::operator/(Float3 vec, float scalar) {
  vec /= scalar;
  return vec;
}

Vu::Math::Float3
Vu::Math::operator-(const Float3& vec) {
  return Float3(-vec.x, -vec.y, -vec.z);
}

float
Vu::Math::length(const Float3& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

float
Vu::Math::lengthSquared(const Float3& vec) {
  return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

Vu::Math::Float3
Vu::Math::normalize(const Float3& vec) {
  float l = length(vec);
  if (l < 1e-6f)
    return Float3(0.0f, 0.0f, 0.0f);
  float invLength = 1.0f / l;
  return Float3(vec.x * invLength, vec.y * invLength, vec.z * invLength);
}

float
Vu::Math::dot(const Float3& a, const Float3& b) {
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vu::Math::Float3
Vu::Math::cross(const Float3& a, const Float3& b) {
  return Float3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vu::Math::Float3
Vu::Math::lerp(const Float3& a, const Float3& b, float t) {
  return Float3(lerp(a.x, b.x, t), lerp(a.y, b.y, t), lerp(a.z, b.z, t));
}
