#pragma once
#include <algorithm>

#include "flecs.h"

#include "Common.h"
#include "VuShader.h"
#include "VuCtx.h"
#include "VuMath.h"
#include "Camera.h"
#include "Transform.h"
#include "Components.h"
#include "VuRenderer.h"
#include "VuMaterialDataPool.h"
#include "VuMathUtils.h"

namespace Vu {
    inline flecs::system AddRenderingSystem(flecs::world& world) {
        return world.system<Transform, const MeshRenderer>("Rendering")
                .each([](Transform& transform, const MeshRenderer& meshRenderer) {

                    auto shader = meshRenderer.shader;
                    auto mat    = shader.get()->materials[meshRenderer.materialIndex];
                    auto adr    = VuMaterialDataPool::mapAddressToBufferDeviceAddress(mat.pbrMaterialData);
                    ctx::vuRenderer->bindMaterial(mat);
                    float4x4 trs = transform.ToTRS();
                    GPU_PushConstant pc{
                        trs,
                        adr,
                        {
                            meshRenderer.mesh->vertexBuffer.index,
                            (uint32) meshRenderer.mesh->vertexCount,
                            0
                        }
                    };
                    ctx::vuRenderer->pushConstants(pc);
                    ctx::vuRenderer->bindMesh(*meshRenderer.mesh);
                    ctx::vuRenderer->drawIndexed(meshRenderer.mesh->indexBuffer.get()->lenght);
                });
    }

    inline flecs::system AddSpinningSystem(flecs::world& world) {

        return world.system<Transform, Spinn>("SpinningSystem")
                .each([](Transform& trs, Spinn& spinn) {
                    trs.Rotate(spinn.axis, spinn.angle * ctx::deltaAsSecond);
                });
    }

    inline flecs::system AddSpinUISystem(flecs::world& world) {

        return world.system<Spinn>("spinUI")
                .each([](flecs::entity e, Spinn& spinn) {

                    if (ImGui::CollapsingHeader("Spin Components")) {
                        ImGui::SliderFloat(std::format("Radians/perSecond##{0}", e.id()).c_str(), &spinn.angle, 0.0f, 32.0f);
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
                                        &trs.position.x, -8.0f, 8.0f);
                    // ImGui::Text(std::format("Rotation {0:}", glm::to_string(trs.rotation)).c_str());
                    // ImGui::Text(std::format("Scale {0:}", glm::to_string(trs.scale)).c_str());
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

                    float3 movement = input * velocity * ctx::deltaAsSecond;
                    //Mouse
                    auto mouseState = SDL_GetMouseState(nullptr, nullptr);

                    bool mouseRightClick = (mouseState & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) != 0;

                    if (mouseRightClick) {
                        // if (!SDL_GetWindowRelativeMouseMode(ctx::sdlWindow)) {
                        //     std::cout << 1 << std::endl;
                        //     SDL_SetWindowRelativeMouseMode(ctx::sdlWindow, true);
                        // }

                        SDL_HideCursor();
                        //SDL_SetWindowMouseGrab(ctx::sdlWindow, true);
                        //if (true) {

                        if (cam.firstClick) {
                            cam.firstClick = false;
                            return;
                        }

                        float xOffset = ctx::mouseDeltaX;
                        float yOffset = ctx::mouseDeltaY;

                        xOffset *= cam.sensitivity;
                        yOffset *= cam.sensitivity;

                        float smoothingFactor = 0.4f; // tweak this value
                        float smoothedDeltaX  = (cam.lastX * smoothingFactor) + (xOffset * (1.0f - smoothingFactor));
                        float smoothedDeltaY  = (cam.lastY * smoothingFactor) + (yOffset * (1.0f - smoothingFactor));

                        cam.yaw +=
                                smoothedDeltaY;
                        cam.pitch +=
                                smoothedDeltaX;

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
                    float3     rotatedTranslation = rotate(asEuler , movement);

                    trs.position.x += rotatedTranslation.x;
                    trs.position.y += rotatedTranslation.y;
                    trs.position.z += rotatedTranslation.z;


                    ctx::frameConst.view = inverse(trs.ToTRS());
                    ctx::frameConst.proj = createPerspectiveProjectionMatrix(
                        cam.fov,
                        static_cast<float>(ctx::vuRenderer->swapChain.swapChainExtent.width),
                        static_cast<float>(ctx::vuRenderer->swapChain.swapChainExtent.height),
                        cam.near,
                        cam.far
                    );

                    ctx::frameConst.cameraPos = float4(trs.position, 0);
                    ctx::frameConst.cameraDir = float4(float3(cam.yaw, cam.pitch, cam.roll), 0);
                    ctx::frameConst.time      = float4(ctx::time(), 0, 0, 0).x;


                    //const auto* state = SDL_GetKeyboardState(nullptr);

                    //ubo.debugIndex = 0;
                    // if (state[SDL_SCANCODE_F1]) {
                    //     ctx::frameConst.debugIndex = 1;
                    // }
                    // if (state[SDL_SCANCODE_F2]) {
                    //     ctx::frameConst.debugIndex = 2;
                    // }
                    // if (state[SDL_SCANCODE_F3]) {
                    //     ctx::frameConst.debugIndex = 3;
                    // }
                    // if (state[SDL_SCANCODE_F4]) {
                    //     ctx::frameConst.debugIndex = 4;
                    // }
                    // if (state[SDL_SCANCODE_F5]) {
                    //     ctx::frameConst.debugIndex = 5;
                    // }
                    // if (state[SDL_SCANCODE_F6]) {
                    //     ctx::frameConst.debugIndex = 6;
                    // }
                    // if (state[SDL_SCANCODE_F7]) {
                    //     ctx::frameConst.debugIndex = 7;
                    // }


                    ctx::vuRenderer->updateFrameConstantBuffer(ctx::frameConst);
                });
    }
}
