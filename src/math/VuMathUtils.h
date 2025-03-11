#pragma once

#include "Common.h"

namespace Vu {
    //
    // inline quaternion mulQuaternions(quaternion q1, quaternion q2) {
    //     float w1 = q1.w(), x1 = q1.x(), y1 = q1.y, z1 = q1.z();
    //     float w2 = q2.w(), x2 = q2.x(), y2 = q2.y, z2 = q2.z();
    //
    //     // Calculate the components of the resulting quaternion
    //     float w = w1 * w2 - x1 * x2 - y1 * y2 - z1 * z2;
    //     float x = w1 * x2 + x1 * w2 + y1 * z2 - z1 * y2;
    //     float y = w1 * y2 - x1 * z2 + y1 * w2 + z1 * x2;
    //     float z = w1 * z2 + x1 * y2 - y1 * x2 + z1 * w2;
    //
    //     // Return the resulting quaternion as a float4
    //     return quaternion(x, y, z, w);
    // }
    //
    // inline float3 quatMul(const quaternion& q, const float3& v) {
    //     // Extract the vector part of the quaternion
    //     float3 q_xyz = float3(q.x(), q.y(), q.z());
    //
    //     // Compute the cross product of q.xyz and v
    //     float3 t = fastgltf::math::cross(q_xyz, v) * 2.0f;
    //
    //     // Return the transformed vector
    //     return fastgltf::math::cross(q_xyz, t) + v + (t * q.w());
    // }
    //
    // inline float4x4 createTRSMatrix(float3 position, quaternion rotation, float3 scale) {
    //     // Normalize the quaternion to ensure a valid rotation
    //     quaternion q = normalize(rotation);
    //     // Extract quaternion components
    //     float x = q.x();
    //     float y = q.y();
    //     float z = q.z();
    //     float w = q.w();
    //     // Calculate rotation matrix (column-major order)
    //     float xx = x * x;
    //     float yy = y * y;
    //     float zz = z * z;
    //
    //     float xy = x * y;
    //     float xz = x * z;
    //     float yz = y * z;
    //
    //     float wx = w * x;
    //     float wy = w * y;
    //     float wz = w * z;
    //
    //     float3 right = float3(1.0F - 2.0F * (yy + zz),
    //                           2.0F * (xy + wz),
    //                           2.0F * (xz - wy));
    //
    //     float3 up = float3(2.0F * (xy - wz),
    //                        1.0F - 2.0F * (xx + zz),
    //                        2.0F * (yz + wx));
    //
    //     float3 forward = float3(2.0F * (xz + wy),
    //                             2.0F * (yz - wx),
    //                             1.0F - 2.0F * (xx + yy));
    //
    //     // Apply scaling to the rotation basis vectors
    //     right   = right * scale.x();
    //     up      = up * scale.y();
    //     forward = forward * scale.z();
    //
    //     // Construct the column-major TRS matrix
    //     float4x4 trsMatrix = float4x4(
    //         right.x(), up.x(), forward.x(), 0.0F,
    //         right.y(), up.y(), forward.y(), 0.0F,
    //         right.z(), up.z(), forward.z(), 0.0F,
    //         position.x(), position.y(), position.z(), 1.0F
    //     );
    //
    //     return trsMatrix;
    // }


    //
    // inline quaternion quaternionFromEulerYXZ(float3 euler) {
    //     // Half angles
    //     float halfYaw   = euler.y * 0.5f; // Y
    //     float halfPitch = euler.x * 0.5f; // X
    //     float halfRoll  = euler.z * 0.5f; // Z
    //
    //     // Sines and cosines of half angles
    //     float sinYaw = sin(halfYaw);
    //     float cosYaw = cos(halfYaw);
    //
    //     float sinPitch = sin(halfPitch);
    //     float cosPitch = cos(halfPitch);
    //
    //     float sinRoll = sin(halfRoll);
    //     float cosRoll = cos(halfRoll);
    //     // Calculate quaternion components (YXZ order)
    //     quaternion q;
    //     q.x = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    //     q.y = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;
    //     q.z = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    //     q.w = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    //     return normalize(q);
    // }

    inline float4x4 createPerspectiveProjectionMatrix(float fovAsRadian, float width, float height, float near, float far) {
        // Calculate the aspect ratio using width and height
        float aspectRatio = width / height;

        // Calculate the tangent of half the field of view angle
        float tanHalfFov = tan(fovAsRadian * 0.5f);

        // Construct the projection matrix
        float4x4 projMatrix = float4x4(
            1.0f / (aspectRatio * tanHalfFov), 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f,
            0.0f, 0.0f, -(far + near) / (far - near), -1.0f,
            0.0f, 0.0f, -(2.0f * far * near) / (far - near), 0.0f
        );

        return projMatrix;
    }

    // static glm::quat safeQuatLookAt(
    //     glm::vec3 const& lookFrom,
    //     glm::vec3 const& lookTo,
    //     glm::vec3 const& up,
    //     glm::vec3 const& alternativeUp) {
    //     glm::vec3 direction       = lookTo - lookFrom;
    //     float     directionLength = glm::length(direction);
    //
    //     // Check if the direction is valid; Also deals with NaN
    //     if (!(directionLength > 0.0001))
    //         return glm::quat(1, 0, 0, 0); // Just return identity
    //
    //     // Normalize direction
    //     direction /= directionLength;
    //
    //     // Is the normal up (nearly) parallel to direction?
    //     if (glm::abs(glm::dot(direction, up)) > .9999f) {
    //         // Use alternative up
    //         return glm::quatLookAt(direction, alternativeUp);
    //     } else {
    //         return glm::quatLookAt(direction, up);
    //     }
    // }
}
