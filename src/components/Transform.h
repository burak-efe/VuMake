#pragma once
#include "Common.h"
#include "glm/gtx/quaternion.hpp"


struct Transform {
    float3 Position = float3(0.0f, 0.0f, 0.0f);
    quat Rotation = glm::identity<quat>();
    float3 Scale = float3(1.0f, 1.0f, 1.0f);


    void Rotate(const float3& axis, float angle) {
        quat rotationQuat = glm::angleAxis(angle, glm::normalize(axis));
        Rotation = rotationQuat * Rotation;
    }

    float4x4 ToTRS() {
        float4x4 transform = float4x4(1.0f);
        transform = glm::translate(transform, Position);
        float4x4 rotationMatrix = glm::toMat4(Rotation);
        transform *= rotationMatrix;
        transform = glm::scale(transform, Scale);
        return transform;
    }

    static quat lookAtQuaternion(const float3& eye, const float3& target, const float3& up) {
        // Compute forward direction (from eye to target)
        float3 forward = glm::normalize(target - eye);

        // Compute right vector
        float3 right = glm::normalize(glm::cross(up, forward));

        // Adjust up vector to ensure orthogonality
        float3 adjustedUp = glm::cross(forward, right);

        // Create rotation matrix from right, adjusted up, and forward vectors
        glm::mat3 rotationMatrix(right, adjustedUp, forward);

        // Convert rotation matrix to quaternion
        return glm::normalize(quat_cast(rotationMatrix));
    }

    // Constructor that sets position, rotation from look-at, and default scale
    static Transform FromLook(const float3& pos, const float3& lookAtPoint,
                              const float3& up = float3(0.0f, 1.0f, 0.0f)) {
        Transform transform{};
        transform.Position = pos;
        transform.Rotation = lookAtQuaternion(pos, lookAtPoint, up);
        return transform;
    }
};
