#pragma once
#include <algorithm>

#include "flecs.h"
#include "GLFW/glfw3.h"

#include "Common.h"
#include "Vu.h"
#include "VuMath.h"
#include "VuRenderer.h"
#include "components/Camera.h"
#include "components/Transform.h"


inline flecs::system AddFlyCameraSystem(flecs::world& world) {

    return world.system<Transform, Camera>("CameraMovement")
            .each([](Transform& trs, Camera& cam) {

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
