#pragma once
#include <cmath>

namespace Vu::Math {
    struct Float3;
    struct Float4;

    struct Quaternion {
        float x;
        float y;
        float z;
        float w;

        // Constructors
        Quaternion();

        constexpr Quaternion(float x, float y, float z, float w);

        // Create identity quaternion (no rotation)
        static constexpr Quaternion identity() {
            Quaternion q;
            q.x = 0.0f;
            q.y = 0.0f;
            q.z = 0.0f;
            q.w = 1.0f;
            return q;
        };

        // Basic operators
        Quaternion& operator*=(const Quaternion& rhs);


        // Conjugate (inverse if normalized)
        Quaternion conjugate() const;

        // Length calculations
        float lengthSquared() const;

        float length() const;

        // Normalize this quaternion
        Quaternion& normalize();

        // Get a normalized copy
        Quaternion normalized() const;

        // Convert to Float4 (x, y, z, w)
        Float4 toFloat4() const;

        // Convert to Euler angles in YXZ order (radians)
        Float3 toEulerYXZ() const;

    };

    // Non-member operators
    Quaternion operator*(Quaternion lhs, const Quaternion& rhs);

    Quaternion operator*(const Quaternion& q, const float s);

    // Dot product
    float dot(const Quaternion& a, const Quaternion& b);

    // Spherical linear interpolation
    Quaternion slerp(const Quaternion& a, const Quaternion& b, float t);

    // Implementation of static methods
    Quaternion fromAxisAngle(const Float3& axis, float angleRadians);

    Quaternion fromEulerYXZ(float yaw, float pitch, float roll);

    Quaternion fromEulerYXZ(const Float3& eulerRadians);

    Quaternion rotateOnAxis(const Quaternion& inputQuat, const Float3& axis, float angleRadians);

    Float3 rotate(const Quaternion& q, const Float3& v);

}
