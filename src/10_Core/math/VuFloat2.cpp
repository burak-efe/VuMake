#include "VuFloat2.h"
#include "VuFloat.h"

#include <cmath>

Vu::Math::Float2::Float2(): x(0.0f), y(0.0f) {
}

Vu::Math::Float2::Float2(float x, float y): x(x), y(y) {
}

Vu::Math::Float2& Vu::Math::Float2::operator+=(const Float2& rhs) {
    x += rhs.x;
    y += rhs.y;
    return *this;
}

Vu::Math::Float2& Vu::Math::Float2::operator-=(const Float2& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    return *this;
}

Vu::Math::Float2& Vu::Math::Float2::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    return *this;
}

Vu::Math::Float2& Vu::Math::Float2::operator/=(float scalar) {
    float invScalar = 1.0f / scalar;
    x *= invScalar;
    y *= invScalar;
    return *this;
}

Vu::Math::Float2 Vu::Math::operator+(Float2 lhs, const Float2& rhs) {
    lhs += rhs;
    return lhs;
}

Vu::Math::Float2 Vu::Math::operator-(Float2 lhs, const Float2& rhs) {
    lhs -= rhs;
    return lhs;
}

Vu::Math::Float2 Vu::Math::operator*(Float2 vec, const float scalar) {
    vec *= scalar;
    return vec;
}

Vu::Math::Float2 Vu::Math::operator*(const float scalar, Float2 vec) {
    vec *= scalar;
    return vec;
}

Vu::Math::Float2 Vu::Math::operator/(Float2 vec, const float scalar) {
    vec /= scalar;
    return vec;
}

Vu::Math::Float2 Vu::Math::operator-(const Float2& vec) {
    return Float2(-vec.x, -vec.y);
}

float Vu::Math::length(const Float2& vec) {
    return std::sqrt(vec.x * vec.x + vec.y * vec.y);
}

float Vu::Math::lengthSquared(const Float2& vec) {
    return vec.x * vec.x + vec.y * vec.y;
}

Vu::Math::Float2 Vu::Math::normalize(const Float2& vec) {
    float l = length(vec);
    if (l < 1e-6f) return Float2(0.0f, 0.0f);
    float invLength = 1.0f / l;
    return Float2(vec.x * invLength, vec.y * invLength);
}

float Vu::Math::dot(const Float2& a, const Float2& b) {
    return a.x * b.x + a.y * b.y;
}

Vu::Math::Float2 Vu::Math::lerp(const Float2& a, const Float2& b, float t) {
    return Float2(
        lerp(a.x, b.x, t),
        lerp(a.y, b.y, t)
    );
}
