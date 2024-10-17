#pragma once
#include <algorithm>

#include "flecs.h"
//#include "GLFW/glfw3.h"

#include "Common.h"
#include "Vu.h"
#include "VuMath.h"
#include "VuRenderer.h"
#include "Camera.h"
#include "Transform.h"
#include "Components.h"


inline flecs::system AddRenderingSystem(flecs::world& world) {
    return world.system<Transform, const MeshRenderer>("Rendering")
            .each([](Transform& trs, const MeshRenderer& meshRenderer) {
                Vu::Renderer->RenderMesh(*meshRenderer.Mesh, trs.ToTRS());
            });
}

inline flecs::system AddSpinningSystem(flecs::world& world) {

    return world.system<Transform, Spinn>("SpinningSystem")
            .each([](Transform& trs, Spinn& spinn) {
                trs.Rotate(spinn.Axis, spinn.Angle * Vu::DeltaAsSecond);
            });
}

inline flecs::system AddSpinUISystem(flecs::world& world) {

    return world.system<Spinn>("spinUI")
            .each([](flecs::entity e, Spinn& spinn) {

                if (ImGui::CollapsingHeader("Spin Components")) {
                    ImGui::SliderFloat(std::format("Radians/perSecond##{0}", e.id()).c_str(), &spinn.Angle, 0.0f, 32.0f);
                }
            });
}

inline flecs::system AddTransformUISystem(flecs::world& world) {
    return world.system<Transform>("trsUI")
            .each([](flecs::iter& it, size_t index, Transform& trs) {


                auto e = it.entity(index);
                // bool open = false;
                // if (index == 0) {
                // }
                // open = ImGui::CollapsingHeader("Transform Components");
                //if (open) {

                 ImGui::Separator();
                 ImGui::Text(e.name());
                 ImGui::SliderFloat3(std::format("Position ##{0}", e.id()).c_str(),
                                     &trs.Position.x, -8.0f, 8.0f);
                 ImGui::Text(std::format("Rotation {0:}", glm::to_string(trs.Rotation)).c_str());
                 ImGui::Text(std::format("Scale {0:}", glm::to_string(trs.Scale)).c_str());
                //}
            });
}

inline flecs::system AddCameraUISystem(flecs::world& world) {

    return world.system<Camera>("camUI")
            .each([](flecs::entity e, Camera& cam) {

                if (ImGui::CollapsingHeader("Camera Components")) {
                    ImGui::Separator();
                    ImGui::Text(e.name());
                    ImGui::SliderFloat(std::format("FOV##{0}", e.id()).c_str(), &cam.fov, 20.0f, 140.0f);

                }
            });

}

// inline flecs::system System(flecs::world& world) {
// }


inline flecs::system AddFlyCameraSystem(flecs::world& world) {

    return world.system<Transform, Camera>("CameraMovement")
            .each([](Transform& trs, Camera& cam) {
                SDL_PumpEvents();

                const auto* state = SDL_GetKeyboardState(nullptr);

                float velocity = cam.cameraSpeed;
                if (state[SDL_SCANCODE_LSHIFT]) {
                    velocity *= 2.0f;
                }


                float3 input{};
                if (state[SDL_SCANCODE_W]) {
                    input.z -= 1;
                }
                if (state[SDL_SCANCODE_S]) {
                    input.z += 1;
                }
                if (state[SDL_SCANCODE_A]) {
                    input.x -= 1;
                }
                if (state[SDL_SCANCODE_D]) {
                    input.x += 1;
                }
                if (state[SDL_SCANCODE_E]) {
                    input.y += 1;
                }
                if (state[SDL_SCANCODE_Q]) {
                    input.y -= 1;
                }

                float3 movement = input * velocity * Vu::DeltaAsSecond;
                //Mouse
                auto mouseState = SDL_GetMouseState(nullptr, nullptr);

                bool mouseRightClick = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;

                if (mouseRightClick) {
                    // if (!SDL_GetWindowRelativeMouseMode(Vu::sdlWindow)) {
                    //     std::cout << 1 << std::endl;
                    //     SDL_SetWindowRelativeMouseMode(Vu::sdlWindow, true);
                    // }

                    SDL_HideCursor();
                    //SDL_SetWindowMouseGrab(Vu::sdlWindow, true);
                    //if (true) {

                    if (cam.firstClick) {
                        cam.firstClick = false;
                        return;
                    }

                    float xOffset = -Vu::MouseDeltaX;
                    float yOffset = -Vu::MouseDeltaY;

                    xOffset *= cam.sensitivity;
                    yOffset *= cam.sensitivity;

                    float smoothingFactor = 0.4f; // tweak this value
                    float smoothedDeltaX = (cam.lastX * smoothingFactor) + (xOffset * (1.0f - smoothingFactor));
                    float smoothedDeltaY = (cam.lastY * smoothingFactor) + (yOffset * (1.0f - smoothingFactor));

                    cam.yaw +=
                            smoothedDeltaY;
                    cam.pitch +=
                            smoothedDeltaX;

                    cam.pitch = std::clamp(cam.pitch, -89.0f, 89.0f);

                    cam.lastX = smoothedDeltaX;
                    cam.lastY = smoothedDeltaY;
                } else {
                    SDL_ShowCursor();
                    // if (SDL_GetWindowRelativeMouseMode(Vu::sdlWindow)) {
                    //     SDL_SetWindowRelativeMouseMode(Vu::sdlWindow, false);
                    // }
                    cam.firstClick = true;
                }


                trs.SetEulerAngles(float3(cam.yaw, cam.pitch, cam.roll));

                glm::quat asEuler = glm::quat(float3(cam.yaw, cam.pitch, cam.roll));
                float3 rotatedTranslation = quat_mul(asEuler, movement);

                trs.Position.x += rotatedTranslation.x;
                trs.Position.y += rotatedTranslation.y;
                trs.Position.z += rotatedTranslation.z;


                FrameUBO ubo{};
                ubo.view = glm::inverse(trs.ToTRS());
                ubo.proj = glm::perspective(
                    glm::radians(cam.fov),
                    static_cast<float>(Vu::Renderer->SwapChain.SwapChainExtent.width)
                    / static_cast<float>(Vu::Renderer->SwapChain.SwapChainExtent.height),
                    cam.near,
                    cam.far);
                Vu::Renderer->UpdateUniformBuffer(ubo);
            });
}
