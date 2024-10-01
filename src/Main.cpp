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
//#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"
//#include <glm/gtc/matrix_transform.hpp>
#include "stb_image.h"
#include "GLFW/glfw3.h"

#include "Vu.h"
#include "VuRenderer.h"
#include "VuMath.h"
#include "Mesh.h"
#include "components/Transform.h"
#include "components/Camera.h"
#include "systems/FlyCamera.h"


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

    auto camSys = AddFlyCameraSystem(world);

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
        camSys.run();

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
