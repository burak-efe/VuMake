#pragma once
#include "02_OuterCore/Common.h"
#include "VuFloat4x4.h"

#include <math.h>

namespace Vu {
inline mat4x4
createPerspectiveProjectionMatrix(float fovAsRadian, float width, float height, float near, float far) {
  // Calculate the aspect ratio using width and height
  float aspectRatio = width / height;

  // Calculate the tangent of half the field of view angle
  float tanHalfFov = ::tan(fovAsRadian * 0.5f);

  // Construct the perspective projection matrix for WebGPU (depth range: [0, 1])
  // The matrix layout is in column-major order.
  mat4x4 projMatrix = mat4x4(
      // X-axis scaling (based on aspect ratio and field of view)
      1.0f / (aspectRatio * tanHalfFov), 0.0f, 0.0f, 0.0f,

      // Y-axis scaling (based on field of view)
      0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f,

      // Z-axis scaling and translation
      // Maps near plane to 0.0 and far plane to 1.0 for WebGPU's NDC range [0, 1]
      0.0f, 0.0f, far / (near - far), -1.0f,

      // Translation along Z-axis
      0.0f, 0.0f, (far * near) / (near - far), 0.0f);

  return projMatrix;
}
} // namespace Vu
