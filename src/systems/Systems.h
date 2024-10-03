#pragma once
#include <algorithm>

#include "flecs.h"
#include "GLFW/glfw3.h"

#include "Common.h"
#include "Vu.h"
#include "VuMath.h"
#include "VuRenderer.h"
#include "Camera.h"
#include "Transform.h"
#include "Components.h"


inline flecs::system RenderingSystem(flecs::world& world) {
    return world.system<Transform, const MeshRenderer>("Rendering")
            .each([](Transform& trs, const MeshRenderer& meshRenderer) {
                Vu::Renderer->RenderMesh(*meshRenderer.Mesh, trs.ToTRS());
            });
}

inline flecs::system SpinningSystem(flecs::world& world) {

    return world.system<Transform, Spinn>("SpinningSystem")
            .each([](Transform& trs, Spinn& spinn) {
                trs.Rotate(spinn.Axis, spinn.Angle * Vu::DeltaTime);
            });
}

inline flecs::system SpinUISystem(flecs::world& world) {

    return world.system<Spinn>("spinUI")
            .each([](flecs::entity e, Spinn& spinn) {

                if (ImGui::CollapsingHeader("Spin Components")) {
                    ImGui::SliderFloat(std::format("Radians/perSecond##{0}", e.id()).c_str(), &spinn.Angle, 0.0f, 32.0f);
                }
            });
}

inline flecs::system TransformUISystem(flecs::world& world) {
    return world.system<Transform>("trsUI")
            .each([](flecs::iter& it, size_t index, Transform& trs) {


                auto e = it.entity(index);
                bool open = false;
                if (index == 0) {
                    open = ImGui::CollapsingHeader("Transform Components");
                }
                if (open) {

                    ImGui::Separator();
                    ImGui::Text(e.name());
                    ImGui::SliderFloat3(std::format("Position ##{0}", e.id()).c_str(),
                                        &trs.Position.x, -8.0f, 8.0f);
                    ImGui::Text(std::format("Rotation {0:}", glm::to_string(trs.Rotation)).c_str());
                    ImGui::Text(std::format("Scale {0:}", glm::to_string(trs.Scale)).c_str());
                }
            });
}

inline flecs::system CameraUISystem(flecs::world& world) {

    return  world.system<Camera>("camUI")
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

                assert(Vu::window != nullptr);
                float velocity = cam.cameraSpeed;
                if (glfwGetKey(Vu::window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                    velocity *= 2.0f;
                }

                float3 input{};
                if (glfwGetKey(Vu::window, GLFW_KEY_W) == GLFW_PRESS) {
                    input.z -= 1;
                }
                if (glfwGetKey(Vu::window, GLFW_KEY_S) == GLFW_PRESS) {
                    input.z += 1;
                }
                if (glfwGetKey(Vu::window, GLFW_KEY_A) == GLFW_PRESS) {
                    input.x -= 1;
                }
                if (glfwGetKey(Vu::window, GLFW_KEY_D) == GLFW_PRESS) {
                    input.x += 1;
                }
                if (glfwGetKey(Vu::window, GLFW_KEY_E) == GLFW_PRESS) {
                    input.y += 1;
                }
                if (glfwGetKey(Vu::window, GLFW_KEY_Q) == GLFW_PRESS) {
                    input.y -= 1;
                }


                float3 movement = input * velocity * Vu::DeltaTime;
                if (glfwGetMouseButton(Vu::window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
                    if (cam.firstClick) {
                        cam.firstClick = false;
                        return;
                    }
                    float xOffset = cam.lastX - Vu::MouseX;
                    float yOffset = cam.lastY - Vu::MouseY; // Reversed since y-coordinates range from bottom to top


                    xOffset *= cam.sensitivity;
                    yOffset *= cam.sensitivity;

                    cam.yaw += yOffset;
                    cam.pitch += xOffset;

                    cam.pitch = std::clamp(cam.pitch, -89.0f, 89.0f);

                } else {
                    cam.firstClick = true;
                }

                cam.lastX = Vu::MouseX;
                cam.lastY = Vu::MouseY;

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
                    static_cast<float>(Vu::Renderer->SwapChain.swapChainExtent.width)
                    / static_cast<float>(Vu::Renderer->SwapChain.swapChainExtent.height),
                    cam.near,
                    cam.far);
                Vu::Renderer->UpdateUniformBuffer(ubo);
            });
}
