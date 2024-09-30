#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_VULKAN

#include <format>
#include <filesystem>
#include "flecs.h"
#include "imgui/imgui.h"
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include "GLFW/glfw3.h"
#include "VuRenderer.h"
#include "Mesh.h"
#include "components/Transform.h"
#include "components/Camera.h"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

static glm::vec3 quat_mul(const glm::quat& q, const glm::vec3& v) {
    // Extract the vector part of the quaternion
    glm::vec3 q_xyz = glm::vec3(q.x, q.y, q.z);

    // Compute the cross product of q.xyz and v
    glm::vec3 t = 2.0f * glm::cross(q_xyz, v);

    // Return the transformed vector
    return v + q.w * t + glm::cross(q_xyz, t);
}


struct MeshRenderer {
    Mesh* Mesh;
};


struct Spinn {
    float3 Axis = float3(0, 1, 0);
    float Angle = glm::radians(360.f);
};



void RunEngine() {

    //Vu::Check(VK_TIMEOUT);
    double prevTime = 0;

    VuRenderer vuRenderer;
    vuRenderer.Init();
    Vu::Renderer = &vuRenderer;
    Mesh monke("shaders/monka500k.glb", Vu::VmaAllocator);

    //auto texture = VuTexture("shaders/uvChecker.png");
    //texture.Dispose();

    flecs::world world;
    world.set<VuRenderer>(vuRenderer);


    world.entity("Monke2").set(Transform{
        .Position = float3(0, 0, 0)
    }).set(MeshRenderer{&monke}).set(Spinn{});

    world.entity("Cam").set(
        Transform(float3(0, 0, 4.0f), float3(0, 0, 0), float3(1, 1, 1))
    ).set(Camera{});

    auto cameraMovementSystem = world.system<Transform, Camera>("CameraMovement")
            .each([](Transform& trs, Camera& cam) {

                float velocity = cam.cameraSpeed;
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                    velocity *= 2.0f;

                float3 input{};
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_W) == GLFW_PRESS)
                    input.z -= 1;
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_S) == GLFW_PRESS)
                    input.z += 1;
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_A) == GLFW_PRESS)
                    input.x -= 1;
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_D) == GLFW_PRESS)
                    input.x += 1;
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_E) == GLFW_PRESS)
                    input.y += 1;
                if (glfwGetKey(Vu::Renderer->window, GLFW_KEY_Q) == GLFW_PRESS)
                    input.y -= 1;


                float3 movement = input * velocity * Vu::DeltaTime;
                if (glfwGetMouseButton(Vu::Renderer->window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
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


    auto renderingSystem = world.system<Transform, const MeshRenderer>("Rendering")
            .each([](Transform& trs, const MeshRenderer& meshRenderer) {
                Vu::Renderer->RenderMesh(*meshRenderer.Mesh, trs.ToTRS());
            });

    auto spinningSystem = world.system<Transform, Spinn>("SpinningSystem")
            .each([](Transform& trs, Spinn& spinn) {
                trs.Rotate(spinn.Axis, spinn.Angle * Vu::DeltaTime);
            });

    auto spinUI = world.system<Spinn>("spinUI")
            .each([](flecs::entity e, Spinn& spinn) {

                if (ImGui::CollapsingHeader("Spin Components")) {
                    ImGui::SliderFloat(std::format("Radians/perSecond##{0}", e.id()).c_str(), &spinn.Angle, 0.0f, 32.0f);
                }
            });

    auto trsUI = world.system<Transform>("trsUI")
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

    auto camUI = world.system<Camera>("camUI")
            .each([](flecs::entity e, Camera& cam) {

                if (ImGui::CollapsingHeader("Camera Components")) {
                    ImGui::Separator();
                    ImGui::Text(e.name());
                    ImGui::SliderFloat(std::format("FOV##{0}", e.id()).c_str(), &cam.fov, 20.0f, 140.0f);

                }
            });


    while (!vuRenderer.ShouldWindowClose()) {
        //deltaTime
        Vu::DeltaTime = (glfwGetTime() - prevTime);
        prevTime = glfwGetTime();

        //System Begins
        spinningSystem.run();
        cameraMovementSystem.run();

        //Rendering Begins
        vuRenderer.BeginFrame();
        renderingSystem.run();

        vuRenderer.BeginImgui();

        ImGui::Text(
            std::format("Frame Per Second: {0:.0f}", (1.0f / Vu::DeltaTime))
            .c_str());
        ImGui::Text(std::format("Frame Time as miliSec: {0:.4}", Vu::DeltaTime * 1000).c_str());
        spinUI.run();
        trsUI.run();
        camUI.run();
        vuRenderer.EndImgui();

        vuRenderer.EndFrame();
    }

    vuRenderer.WaitIdle();
    monke.Dispose();
    vuRenderer.Dispose();
    system("pause");
}


int main() {
    RunEngine();
    return EXIT_SUCCESS;
}
