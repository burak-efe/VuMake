#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_VULKAN

#include <format>
#include <filesystem>
#include "flecs.h"
#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include "GLFW/glfw3.h"
#include "VuRenderer.h"
#include "Mesh.h"
#include "components/Transform.h"
#include "components/Camera.h"

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
    VuContext::Renderer = &vuRenderer;
    Mesh monke("shaders/monka500k.glb", VuContext::VmaAllocator);

    //auto texture = VuTexture("shaders/uvChecker.png");
    //texture.Dispose();

    flecs::world world;
    world.set<VuRenderer>(vuRenderer);


    world.entity("Monke2").set(Transform{
        .Position = float3(0, 0, 0)
    }).set(MeshRenderer{&monke}).set(Spinn{});

    world.entity("Cam").set(
        Transform::FromLook(float3(0, 0, -4.0f), float3(0, 0, 0), float3(0, 1, 0))
    ).set(Camera{});

    auto cameraMovementSystem = world.system<Transform, Camera>("CameraMovement")
            .each([](Transform& trs, Camera& cam) {

                float velocity = cam.cameraSpeed * VuContext::DeltaTime;
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                    velocity *= 2.0f;
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_W) == GLFW_PRESS)
                    trs.Position += velocity * cam.cameraFront;
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_S) == GLFW_PRESS)
                    trs.Position -= velocity * cam.cameraFront;
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_A) == GLFW_PRESS)
                    trs.Position -= glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp)) * velocity;
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_D) == GLFW_PRESS)
                    trs.Position += glm::normalize(glm::cross(cam.cameraFront, cam.cameraUp)) * velocity;
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_E) == GLFW_PRESS)
                    trs.Position += velocity * cam.cameraUp; // Move up
                if (glfwGetKey(VuContext::Renderer->window, GLFW_KEY_Q) == GLFW_PRESS)
                    trs.Position -= velocity * cam.cameraUp; // Move down


                if (glfwGetMouseButton(VuContext::Renderer->window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
                    if (cam.firstClick) {

                        cam.firstClick = false;
                        return;
                    } else {
                        float xOffset = VuContext::MouseX - cam.lastX;
                        float yOffset = cam.lastY - VuContext::MouseY; // Reversed since y-coordinates range from bottom to top
                        cam.lastX = VuContext::MouseX;
                        cam.lastY = VuContext::MouseY;

                        xOffset *= cam.sensitivity;
                        yOffset *= cam.sensitivity;

                        cam.yaw += xOffset;
                        cam.pitch += yOffset;

                        // Constrain the pitch to avoid camera flipping
                        if (cam.pitch > 89.0f)
                            cam.pitch = 89.0f;
                        if (cam.pitch<-89.0f)
                                cam.pitch = -89.0f;

                                // Calculate the new front vector
                            float3 front;
                                front.x = cos(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
                        front.y = sin(glm::radians(cam.pitch));
                        front.z = sin(glm::radians(cam.yaw)) * cos(glm::radians(cam.pitch));
                        cam.cameraFront = glm::normalize(front);
                    }
                } else {
                    cam.lastX = VuContext::MouseX;
                    cam.lastY = VuContext::MouseY;
                    cam.firstClick = true;
                }

                //trs.Rotation = Transform::lookAtQuaternion(trs.Position, cam.cameraFront, cam.cameraUp);

                FrameUBO ubo{};
                ubo.view = glm::lookAt(trs.Position, trs.Position + cam.cameraFront, cam.cameraUp);
                //trs.ToTRS();
                //glm::lookAt(float3(0.0f, 0.0f, 3.0f), float3(0.0f, 0.0f, 0.0f), float3(0.0f, 1.0f, 0.0f));

                ubo.proj = glm::perspective(
                    glm::radians(cam.fov),
                    static_cast<float>(VuContext::Renderer->SwapChain.swapChainExtent.width)
                    / static_cast<float>(VuContext::Renderer->SwapChain.swapChainExtent.height),
                    cam.near,
                    cam.far);

                ubo.proj[1][1] *= -1;
                VuContext::Renderer->UpdateUniformBuffer(ubo);
            });


    auto renderingSystem = world.system<Transform, const MeshRenderer>("Rendering")
            .each([](Transform& trs, const MeshRenderer& meshRenderer) {
                VuContext::Renderer->RenderMesh(*meshRenderer.Mesh, trs.ToTRS());
            });

    auto spinningSystem = world.system<Transform, Spinn>("SpinningSystem")
            .each([](Transform& trs, Spinn& spinn) {
                trs.Rotate(spinn.Axis, spinn.Angle * VuContext::DeltaTime);
            });

    auto spinUI = world.system<Spinn>("spinUI")
            .each([](flecs::entity e, Spinn& spinn) {

                ImGui::SliderFloat(std::format("Radians/perSecond##{0}", e.id()).c_str(), &spinn.Angle, 0.0f, 32.0f);
            });


    while (!vuRenderer.ShouldWindowClose()) {
        //deltaTime
        VuContext::DeltaTime = (glfwGetTime() - prevTime);
        prevTime = glfwGetTime();

        //System Begins
        spinningSystem.run();
        cameraMovementSystem.run();

        //Rendering Begins
        vuRenderer.BeginFrame();
        //vuRenderer.UpdateUniformBuffer();
        renderingSystem.run();

        vuRenderer.BeginImgui();
        //ImGui::ShowDemoWindow();

        ImGui::Text(
            std::format("Frame Per Second: {0:.0f}", (1.0f / VuContext::DeltaTime))
            .c_str());
        ImGui::Text(std::format("Frame Time as miliSec: {0:.4}", VuContext::DeltaTime * 1000).c_str());
        spinUI.run();
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
