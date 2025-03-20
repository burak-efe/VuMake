#pragma once

namespace Vu {

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

}
