#pragma once

#include "02_OuterCore/math/VuFloat.h"
#include "Transform.h"

#define CONCAT(lhs, rhs)         lhs #rhs
#define CONCAT_WRAPPER(lhs, rhs) CONCAT(lhs, rhs)
#define UNIQUE                   CONCAT_WRAPPER(__FILE__, __LINE__)

namespace Vu {

struct Camera {
  float fov = Math::toRadians(90.0f);

  float near = 0.01f;
  float far  = 100.0f;

  float cameraSpeed = 5.0f;
  float sensitivity = 0.005f;

  float yaw   = 0.0f;
  float pitch = 0.0f;
  float roll  = 0.0f;

  float lastX = 0;
  float lastY = 0;

  bool firstClick = true;
};

inline void
drawCameraUI(GPU::Camera& camera, Transform& camTrs) {
  ImGui::Separator();
  ImGui::Text("Camera");
  ImGui::DragFloat3("Position##01", &camTrs.m_position.x, 0.01f, -999.0f, 999.0f);
  ImGui::DragFloat("Exposure##01", &camera.exposureScale, 0.01f, 0.0f, 10.0f);
  ImGui::Separator();
}

inline void
drawPointLightUi(GPU::PointLight& pointLight, uint32_t id) {
  ImGui::Separator();
  ImGui::Text("PointLight");
  ImGui::DragFloat3(std::format("Position##{}{}", UNIQUE, id).c_str(), &pointLight.position.x, 0.01f, -999.0f, 999.0f);
  ImGui::DragFloat3(std::format("Color##{}{}", UNIQUE, id).c_str(), &pointLight.color.x, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat(std::format("Intensity##{}{}", UNIQUE, id).c_str(), &pointLight.intensity, 0.01f, 0.0f, 10.0f);
  ImGui::DragFloat(std::format("Range##{}{}", UNIQUE, id).c_str(), &pointLight.range, 0.01f, 0.0f, 10.0f);
  ImGui::Separator();
}

} // namespace Vu
