#pragma once
#include <algorithm>

#include "02_OuterCore/Common.h"
#include "02_OuterCore/math/VuFloat3.h"
#include "02_OuterCore/math/VuMathMatrix.h"
#include "04_Crust/VuRenderer.h"
#include "11_Components/Camera.h"
#include "11_Components/Components.h"
#include "11_Components/Transform.h"
#include "imgui.h"

namespace Vu {

inline void
drawMesh(VuRenderer& vuRenderer, Transform& transform, const MeshRenderer& meshRenderer) {

  // VuDevice* vuDevice = &ECS_VU_RENDERER->vuDevice;

  std::shared_ptr<VuMaterial> materialHnd = meshRenderer.materialHnd;

  VuMaterial*          matPtr       = materialHnd.get();
  VuMaterialDataHandle matDataIndex = *matPtr->materialDataHnd;

  // bind pipeline
  vuRenderer.bindMaterial(materialHnd);

  // push constant
  mat4x4           trs = transform.ToTRS();
  GPU_PushConstant pc {
      trs, matDataIndex, {meshRenderer.mesh->vertexBuffer->bindlessIndex, meshRenderer.mesh->vertexCount, 0}};
  vuRenderer.pushConstants(pc);
  vuRenderer.bindMesh(*meshRenderer.mesh);
  uint32_t indexCount = meshRenderer.mesh->indexBuffer->sizeInBytes / 4;
  vuRenderer.drawIndexed(indexCount);
}

inline void
spinn(const VuRenderer& vuRenderer, Transform& trs, const Spinn& spin) {

  trs.Rotate(spin.axis, spin.angle * vuRenderer.deltaAsSecond);
}

inline void
drawSpinUI(uint32_t elemID, Spinn& spinn) {

  if (ImGui::CollapsingHeader("Spin Components")) {
    ImGui::SliderFloat(std::format("Radians/perSecond##{0}", elemID).c_str(), &spinn.angle, 0.0f, 32.0f);
  }
}

// inline flecs::system AddTransformUISystem(flecs::world& world)
// {
//     return world.system<Transform>("trsUI")
//                 .each([](flecs::iter& it, size_t index, Transform& trs)
//                 {
//                     auto e = it.entity(index);
//                     // bool open = false;
//                     // if (index == 0) {
//                     // }
//                     // open = ImGui::CollapsingHeader("Transform Components");
//                     //if (open) {
//
//                     ImGui::Separator();
//                     ImGui::Text("%s", e.name().c_str());
//                     ImGui::SliderFloat3(std::format("Position ##{0}", e.id()).c_str(),
//                                         &trs.position.x, -8.0f, 8.0f);
//                     // ImGui::Text(std::format("Rotation {0:}", glm::to_string(trs.rotation)).c_str());
//                     // ImGui::Text(std::format("Scale {0:}", glm::to_string(trs.scale)).c_str());
//                     //}
//                 });
// }
//
// inline flecs::system AddCameraUISystem(flecs::world& world)
// {
//     return world.system<Camera>("camUI")
//                 .each([](flecs::entity e, Camera& cam)
//                 {
//                     if (ImGui::CollapsingHeader("Camera Components"))
//                     {
//                         ImGui::Separator();
//                         ImGui::Text("%s\n", e.name().c_str());
//                         ImGui::SliderFloat(std::format("FOV##{0}", e.id()).c_str(), &cam.fov, 20.0f, 140.0f);
//                     }
//                 });
// }

inline void
cameraFlySystem(VuRenderer& vuRenderer, Transform& trs, Camera& cam) {

  SDL_PumpEvents();

  const auto* state = SDL_GetKeyboardState(nullptr);

  float velocity = cam.cameraSpeed;
  if (state[SDL_SCANCODE_LSHIFT]) { velocity *= 2.0f; }

  vec3 input {};
  if (state[SDL_SCANCODE_W]) { input.z -= 1; }
  if (state[SDL_SCANCODE_S]) { input.z += 1; }
  if (state[SDL_SCANCODE_A]) { input.x -= 1; }
  if (state[SDL_SCANCODE_D]) { input.x += 1; }
  if (state[SDL_SCANCODE_E]) { input.y += 1; }
  if (state[SDL_SCANCODE_Q]) { input.y -= 1; }

  vec3 movement   = input * velocity * vuRenderer.deltaAsSecond;
  // Mouse
  auto mouseState = SDL_GetMouseState(nullptr, nullptr);

  bool mouseRightClick = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;

  if (mouseRightClick) {
    // if (!SDL_GetWindowRelativeMouseMode(ctx::sdlWindow)) {
    //     std::cout << 1 << std::endl;
    //     SDL_SetWindowRelativeMouseMode(ctx::sdlWindow, true);
    // }

    SDL_HideCursor();
    // SDL_SetWindowMouseGrab(ctx::sdlWindow, true);
    // if (true) {

    if (cam.firstClick) {
      cam.firstClick = false;
      return;
    }

    float xOffset = vuRenderer.mouseDeltaX;
    float yOffset = vuRenderer.mouseDeltaY;

    xOffset *= cam.sensitivity;
    yOffset *= cam.sensitivity;

    float smoothingFactor = 0.4f; // tweak this value
    float smoothedDeltaX  = (cam.lastX * smoothingFactor) + (xOffset * (1.0f - smoothingFactor));
    float smoothedDeltaY  = (cam.lastY * smoothingFactor) + (yOffset * (1.0f - smoothingFactor));

    cam.yaw += smoothedDeltaY;
    cam.pitch += smoothedDeltaX;

    cam.pitch = std::clamp(cam.pitch, -89.0f, 89.0f);

    cam.lastX = smoothedDeltaX;
    cam.lastY = smoothedDeltaY;
  } else {
    SDL_ShowCursor();
    // if (SDL_GetWindowRelativeMouseMode(ctx::sdlWindow)) {
    //     SDL_SetWindowRelativeMouseMode(ctx::sdlWindow, false);
    // }
    cam.firstClick = true;
  }

  trs.SetEulerAngles(vec3(cam.yaw, cam.pitch, cam.roll));

  quaternion asEuler            = fromEulerYXZ(vec3(cam.yaw, cam.pitch, cam.roll));
  vec3       rotatedTranslation = rotate(asEuler, movement);

  trs.position.x += rotatedTranslation.x;
  trs.position.y += rotatedTranslation.y;
  trs.position.z += rotatedTranslation.z;

  vuRenderer.frameConst.view = inverse(trs.ToTRS());

  vuRenderer.frameConst.proj = createPerspectiveProjectionMatrix(
      cam.fov, static_cast<float>(vuRenderer.deferredRenderSpace.vuSwapChain.extend2D.width),
      static_cast<float>(vuRenderer.deferredRenderSpace.vuSwapChain.extend2D.height), cam.near, cam.far);

  vuRenderer.frameConst.inverseView = trs.ToTRS();
  vuRenderer.frameConst.inverseProj = Math::inverse(vuRenderer.frameConst.proj);

  vuRenderer.frameConst.cameraPos = vec4(trs.position, 0);
  vuRenderer.frameConst.cameraDir = vec4(vec3(cam.yaw, cam.pitch, cam.roll), 0);
  vuRenderer.frameConst.time      = vec4(vuRenderer.time(), 0, 0, 0).x;

  // const auto* state = SDL_GetKeyboardState(nullptr);

  // ubo.debugIndex = 0;
  //  if (state[SDL_SCANCODE_F1]) {
  //      ctx::frameConst.debugIndex = 1;
  //  }
  //  if (state[SDL_SCANCODE_F2]) {
  //      ctx::frameConst.debugIndex = 2;
  //  }
  //  if (state[SDL_SCANCODE_F3]) {
  //      ctx::frameConst.debugIndex = 3;
  //  }
  //  if (state[SDL_SCANCODE_F4]) {
  //      ctx::frameConst.debugIndex = 4;
  //  }
  //  if (state[SDL_SCANCODE_F5]) {
  //      ctx::frameConst.debugIndex = 5;
  //  }
  //  if (state[SDL_SCANCODE_F6]) {
  //      ctx::frameConst.debugIndex = 6;
  //  }
  //  if (state[SDL_SCANCODE_F7]) {
  //      ctx::frameConst.debugIndex = 7;
  //  }

  vuRenderer.updateFrameConstantBuffer(vuRenderer.frameConst);
}
} // namespace Vu
