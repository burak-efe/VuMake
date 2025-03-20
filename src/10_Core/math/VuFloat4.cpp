#include "VuFloat4.h"

#include "VuFloat.h"
#include "VuFloat2.h"
#include "VuFloat3.h"
#include <cmath>

Vu::Math::Float4::Float4(): x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
}

Vu::Math::Float4::Float4(float x, float y, float z, float w): x(x), y(y), z(z), w(w)
{
}

Vu::Math::Float4::Float4(const Float3& xyz, float w): x(xyz.x), y(xyz.y), z(xyz.z), w(w)
{
}

Vu::Math::Float4::Float4(const Float2& xy, const Float2& zw): x(xy.x), y(xy.y), z(zw.x), w(zw.y)
{
}

Vu::Math::Float4& Vu::Math::Float4::operator+=(const Float4& rhs)
{
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    w += rhs.w;
    return *this;
}

Vu::Math::Float4& Vu::Math::Float4::operator-=(const Float4& rhs)
{
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    w -= rhs.w;
    return *this;
}

Vu::Math::Float4& Vu::Math::Float4::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vu::Math::Float4& Vu::Math::Float4::operator/=(float scalar)
{
    float invScalar = 1.0f / scalar;
    x *= invScalar;
    y *= invScalar;
    z *= invScalar;
    w *= invScalar;
    return *this;
}

Vu::Math::Float3 Vu::Math::Float4::xyz() const
{
    return Float3(x, y, z);
}

Vu::Math::Float2 Vu::Math::Float4::xy() const
{
    return Float2(x, y);
}

Vu::Math::Float4 Vu::Math::operator+(Float4 lhs, const Float4& rhs)
{
    lhs += rhs;
    return lhs;
}

Vu::Math::Float4 Vu::Math::operator-(Float4 lhs, const Float4& rhs)
{
    lhs -= rhs;
    return lhs;
}

Vu::Math::Float4 Vu::Math::operator*(Float4 vec, float scalar)
{
    vec *= scalar;
    return vec;
}

Vu::Math::Float4 Vu::Math::operator*(float scalar, Float4 vec)
{
    vec *= scalar;
    return vec;
}

Vu::Math::Float4 Vu::Math::operator/(Float4 vec, float scalar)
{
    vec /= scalar;
    return vec;
}

Vu::Math::Float4 Vu::Math::operator-(const Float4& vec)
{
    return Float4(-vec.x, -vec.y, -vec.z, -vec.w);
}

float Vu::Math::length(const Float4& vec)
{
    return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
}

float Vu::Math::lengthSquared(const Float4& vec)
{
    return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w;
}

Vu::Math::Float4 Vu::Math::normalize(const Float4& vec)
{
    float l = length(vec);
    if (l < 1e-6f) return Float4(0.0f, 0.0f, 0.0f, 0.0f);
    float invLength = 1.0f / l;
    return Float4(vec.x * invLength, vec.y * invLength, vec.z * invLength, vec.w * invLength);
}

float Vu::Math::dot(const Float4& a, const Float4& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vu::Math::Float4 Vu::Math::lerp(const Float4& a, const Float4& b, float t)
{
    return Float4(
                  lerp(a.x, b.x, t),
                  lerp(a.y, b.y, t),
                  lerp(a.z, b.z, t),
                  lerp(a.w, b.w, t)
                 );
}
