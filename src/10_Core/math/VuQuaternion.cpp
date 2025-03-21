#include "VuQuaternion.h"

#include "VuFloat3.h"
#include "VuFloat4.h"


namespace Vu::Math {

    Quaternion::Quaternion(): x(0.0f), y(0.0f), z(0.0f), w(1.0f) {
    }



    Quaternion& Quaternion::operator*=(const Quaternion& rhs) {
        float newW = w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z;
        float newX = w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y;
        float newY = w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x;
        float newZ = w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w;

        x = newX;
        y = newY;
        z = newZ;
        w = newW;

        return *this;
    }


    Quaternion Quaternion::conjugate() const {
        return Quaternion(-x, -y, -z, w);
    }

    float Quaternion::lengthSquared() const {
        return x * x + y * y + z * z + w * w;
    }

    float Quaternion::length() const {
        return std::sqrt(lengthSquared());
    }

    Quaternion& Quaternion::normalize() {
        float len = length();
        if (len > 0.0001f) {
            float invLen = 1.0f / len;
            x *= invLen;
            y *= invLen;
            z *= invLen;
            w *= invLen;
        }
        return *this;
    }

    Quaternion Quaternion::normalized() const {
        Quaternion q = *this;
        q.normalize();
        return q;
    }

    Float4 Quaternion::toFloat4() const {
        return Float4(x, y, z, w);
    }

    Float3 Quaternion::toEulerYXZ() const {
        // Convert quaternion to Euler angles in YXZ order
        Float3 euler;

        // Prepare commonly used terms
        float xx = x * x;
        float yy = y * y;
        float zz = z * z;
        float ww = w * w;

        // Pitch (X-axis rotation)
        float sinp = 2.0f * (w * x - y * z);
        if (std::abs(sinp) >= 1.0f) {
            // Use 90 degrees if out of range
            euler.x = std::copysign(3.14159265f / 2.0f, sinp);
        } else {
            euler.x = std::asin(sinp);
        }

        // Yaw (Y-axis rotation)
        float siny_cosp = 2.0f * (w * y + x * z);
        float cosy_cosp = ww - xx - yy + zz;
        euler.y         = std::atan2(siny_cosp, cosy_cosp);

        // Roll (Z-axis rotation)
        float sinr_cosp = 2.0f * (w * z + x * y);
        float cosr_cosp = ww + xx - yy - zz;
        euler.z         = std::atan2(sinr_cosp, cosr_cosp);

        return euler;
    }

    Quaternion operator*(Quaternion lhs, const Quaternion& rhs) {
        lhs *= rhs;
        return lhs;
    }

    Quaternion operator*(const Quaternion& q, float s) {
        return Quaternion{q.x * s, q.y * s, q.z * s, q.w * s};
    }

    float dot(const Quaternion& a, const Quaternion& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    Quaternion slerp(const Quaternion& a, const Quaternion& b, float t) {
        // Compute the cosine of the angle between quaternions
        float d = dot(a, b);

        // If the dot product is negative, slerp won't take the shorter path
        // Fix by inverting one quaternion
        Quaternion end = b;
        if (d < 0.0f) {
            end.x = -end.x;
            end.y = -end.y;
            end.z = -end.z;
            end.w = -end.w;
            d     = -d;
        }

        // If the inputs are too close for comfort, linearly interpolate
        constexpr float DOT_THRESHOLD = 0.9995f;
        if (d > DOT_THRESHOLD) {
            Quaternion result(
                a.x + t * (end.x - a.x),
                a.y + t * (end.y - a.y),
                a.z + t * (end.z - a.z),
                a.w + t * (end.w - a.w)
            );
            return result.normalized();
        }

        // Calculate actual slerp
        float theta0 = std::acos(d);
        float theta  = theta0 * t;

        float sinTheta  = std::sin(theta);
        float sinTheta0 = std::sin(theta0);

        float s0 = std::cos(theta) - d * sinTheta / sinTheta0;
        float s1 = sinTheta / sinTheta0;

        return Quaternion(
            s0 * a.x + s1 * end.x,
            s0 * a.y + s1 * end.y,
            s0 * a.z + s1 * end.z,
            s0 * a.w + s1 * end.w
        );
    }

    Quaternion fromAxisAngle(const Float3& axis, float angleRadians) {
        float halfAngle = angleRadians * 0.5f;
        float s         = std::sin(halfAngle);

        return Quaternion(
            axis.x * s,
            axis.y * s,
            axis.z * s,
            std::cos(halfAngle)
        );
    }

    Quaternion fromEulerYXZ(float yaw, float pitch, float roll) {
        // Calculate half angles
        float halfYaw   = yaw * 0.5f;
        float halfPitch = pitch * 0.5f;
        float halfRoll  = roll * 0.5f;

        // Calculate sin/cos of half angles
        float sinYaw   = std::sin(halfYaw);
        float cosYaw   = std::cos(halfYaw);
        float sinPitch = std::sin(halfPitch);
        float cosPitch = std::cos(halfPitch);
        float sinRoll  = std::sin(halfRoll);
        float cosRoll  = std::cos(halfRoll);

        // Combine rotations for YXZ order
        Quaternion q;
        q.x = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
        q.y = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
        q.z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
        q.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;

        return q;
    }

    Quaternion fromEulerYXZ(const Float3& eulerRadians) {
        return fromEulerYXZ(eulerRadians.y, eulerRadians.x, eulerRadians.z);
    }

    Quaternion rotateOnAxis(const Quaternion& inputQuat, const Float3& axis, float angleRadians) {
        // Normalize the axis to ensure a valid rotation
        Float3 normAxis = Math::normalize(axis);

        // Half-angle for quaternion rotation
        float halfAngle = angleRadians * 0.5F;

        // Compute sin/cos of half the angle
        float sinHalf = sin(halfAngle);
        float cosHalf = cos(halfAngle);

        // Create the rotation quaternion from axis-angle
        Quaternion rotationQuat = Quaternion(
            normAxis.x * sinHalf,
            normAxis.y * sinHalf,
            normAxis.z * sinHalf,
            cosHalf
        );
        // Multiply quaternions: rotation * input (order matters!)
        return rotationQuat * inputQuat;
    }

    Float3 rotate(const Quaternion& q, const Float3& v) {
        Float3 u{q.x, q.y, q.z};
        float  s = q.w;

        Float3 uv  = cross(u, v);
        Float3 uuv = cross(u, uv);

        uv  = uv * (2.0f * s);
        uuv = uuv * 2.0f;

        return v + uv + uuv;
    }

} // namespace VuMath
