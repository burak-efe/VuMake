#include "Systems.h"

#include "02_OuterCore/math/VuMathMatrix.h"
#include "03_Mantle/VuBuffer.h"
#include "04_Crust/VuMaterial.h"
#include "04_Crust/VuMesh.h"
#include "04_Crust/VuRenderer.h"
#include "11_Components/Camera.h"
#include "11_Components/Components.h"
#include "11_Components/Transform.h"
#include "imgui.h"
#include "InteroptStructs.h"
void
Vu::drawMesh(VuRenderer& vuRenderer, Transform& transform, const MeshRenderer& meshRenderer) {

  std::shared_ptr<VuMaterial> materialHnd = meshRenderer.materialHnd;

  VuMaterial*          matPtr       = materialHnd.get();
  VuMaterialDataHandle matDataIndex = *matPtr->m_materialDataHnd;

  // bind pipeline
  vuRenderer.bindMaterial(materialHnd);

  // push constant
  float4x4           trs = transform.ToTRS();
  PushConsts_RawData pc {
      trs,
      matDataIndex,
      {meshRenderer.mesh->m_vertexBuffer->m_bindlessIndex.value_or_THROW(), meshRenderer.mesh->m_vertexCount, 0}};
  vuRenderer.pushConstants(pc);
  vuRenderer.bindMesh(*meshRenderer.mesh);
  uint32_t indexCount = meshRenderer.mesh->m_indexBuffer->m_sizeInBytes / 4;
  vuRenderer.drawIndexed(indexCount);
}
void
Vu::spinn(const VuRenderer& vuRenderer, Transform& trs, const Spinn& spin) {

  trs.Rotate(spin.axis, spin.angle * vuRenderer.m_deltaAsSecond);
}
void
Vu::drawSpinUI(uint32_t elemID, Spinn& spinn) {

  if (ImGui::CollapsingHeader("Spin Components")) {
    ImGui::SliderFloat(std::format("Radians/perSecond##{0}", elemID).c_str(), &spinn.angle, 0.0f, 32.0f);
  }
}
void
Vu::cameraFlySystem(VuRenderer& vuRenderer, Transform& trs, Camera& cam) {

  SDL_PumpEvents();

  const auto* state = SDL_GetKeyboardState(nullptr);

  float velocity = cam.cameraSpeed;
  if (state[SDL_SCANCODE_LSHIFT]) { velocity *= 2.0f; }

  float3 input {};
  if (state[SDL_SCANCODE_W]) { input.z -= 1; }
  if (state[SDL_SCANCODE_S]) { input.z += 1; }
  if (state[SDL_SCANCODE_A]) { input.x -= 1; }
  if (state[SDL_SCANCODE_D]) { input.x += 1; }
  if (state[SDL_SCANCODE_E]) { input.y += 1; }
  if (state[SDL_SCANCODE_Q]) { input.y -= 1; }

  float3 movement = input * velocity * vuRenderer.m_deltaAsSecond;
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

    float xOffset = vuRenderer.m_mouseDeltaX;
    float yOffset = vuRenderer.m_mouseDeltaY;

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

  trs.SetEulerAngles(float3(cam.yaw, cam.pitch, cam.roll));

  quaternion asEuler            = fromEulerYXZ(float3(cam.yaw, cam.pitch, cam.roll));
  float3     rotatedTranslation = rotate(asEuler, movement);

  trs.m_position.x += rotatedTranslation.x;
  trs.m_position.y += rotatedTranslation.y;
  trs.m_position.z += rotatedTranslation.z;

  vuRenderer.m_frameConst.view = inverse(trs.ToTRS());

  vuRenderer.m_frameConst.proj =
      createPerspectiveProjectionMatrix(cam.fov,
                                        static_cast<float>(vuRenderer.m_deferredRenderSpace.m_vuSwapChain.m_extend2D.width),
                                        static_cast<float>(vuRenderer.m_deferredRenderSpace.m_vuSwapChain.m_extend2D.height),
                                        cam.near,
                                        cam.far);

  vuRenderer.m_frameConst.inverseView = trs.ToTRS();
  vuRenderer.m_frameConst.inverseProj = Math::inverse(vuRenderer.m_frameConst.proj);

  vuRenderer.m_frameConst.cameraPos = float4(trs.m_position, 0);
  vuRenderer.m_frameConst.cameraDir = float4(float3(cam.yaw, cam.pitch, cam.roll), 0);
  vuRenderer.m_frameConst.time      = float4(vuRenderer.time(), 0, 0, 0).x;

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

  vuRenderer.updateFrameConstantBuffer(vuRenderer.m_frameConst);
}