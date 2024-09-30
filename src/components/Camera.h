#pragma once
#include "Common.h"

struct Camera {
    float fov = 90.0f;

    float near = 0.01f;
    float far = 100.0f;

    float cameraSpeed = 5.0f;
    float sensitivity = 0.01f;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    float lastX = 0;
    float lastY = 0;

    bool firstClick = true;

    // void UpdateTransform(Transform t)
    // {
    //     t.eulerAngles = new Vector3(pitch, yaw, roll);
    //     t.position = new Vector3(x, y, z);
    // }


    // svoid Translate(float3 translation) {
    //     glm::quat asEuler = glm::quat(float3(yaw,pitch,roll));
    //     float3 rotatedTranslation = quat_mul(asEuler, translation);
    //
    //     x += rotatedTranslation.x;
    //     y += rotatedTranslation.y;
    //     z += rotatedTranslation.z;
    // }




    // void SetYawAndPitchFromQuaternion(const glm::quat& quaternion) {
    //
    //     glm::vec3 eulerAngles = glm::eulerAngles(quaternion);
    //     yaw = glm::degrees(eulerAngles.y);
    //     pitch = glm::degrees(eulerAngles.x);
    // }


};
