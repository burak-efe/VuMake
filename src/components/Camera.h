#pragma once
#include "Common.h"

struct Camera {
    float fov = 90.0f;
    float near = 0.01f;
    float far = 100.0f;
    float cameraSpeed = 5.0f;
    float sensitivity = 0.2f;
    float yaw = 90.0f; // Yaw is initialized to point along the negative Z axis
    float pitch = 0.0f;
    float lastX = 0, lastY = 0;
    bool firstClick = true;

    // Camera attributes
    //float3 cameraPos = float3(0.0f, 0.0f, 3.0f);
    float3 cameraFront = float3(0.0f, 0.0f, 1.0f);
    float3 cameraUp = float3(0.0f, 1.0f, 0.0f);

    void SetYawAndPitchFromQuaternion(const glm::quat& quaternion) {
        // Convert quaternion to Euler angles (GLM uses radians)
        glm::vec3 eulerAngles = glm::eulerAngles(quaternion);

        // Extract yaw and pitch from the Euler angles
        yaw = glm::degrees(eulerAngles.y); // Yaw is the rotation around the Y-axis
        pitch = glm::degrees(eulerAngles.x); // Pitch is the rotation around the X-axis
    }


};