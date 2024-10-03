#define STB_IMAGE_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_VULKAN

#include <filesystem>
#include <format>

#include "flecs.h"
#include "imgui.h"
#include "GLFW/glfw3.h"
#include "stb_image.h"
#include "glm/gtx/string_cast.hpp"


#include "Vu.h"

#include "VuRenderer.h"
#include "Camera.h"
#include "Transform.h"
#include "Mesh.h"
#include "Systems.h"
#include "Components.h"


void RunEngine() {
    // double prevTime = 0;

    VuRenderer vuRenderer;
    vuRenderer.Init();
    Vu::Renderer = &vuRenderer;
    Mesh monke("shaders/monka500k.glb", Vu::VmaAllocator);

    //auto texture = VuTexture("shaders/uvChecker.png");
    //texture.Dispose();

    flecs::world world;
    world.set<VuRenderer>(vuRenderer);

    //Add Systems
    auto spinningSystem = SpinningSystem(world);
    auto flyCameraSystem = AddFlyCameraSystem(world);
    auto renderingSystem = RenderingSystem(world);
    auto spinUI = SpinUISystem(world);
    auto trsUI = TransformUISystem(world);
    auto camUI = CameraUISystem(world);


    //Add Entites
    world.entity("Monke2").set(Transform{
        .Position = float3(0, 0, 0)
    }).set(MeshRenderer{&monke}).set(Spinn{});

    world.entity("Cam").set(
        Transform(float3(0, 0, 4.0f), float3(0, 0, 0), float3(1, 1, 1))
    ).set(Camera{});


    //Update Loop
    while (!vuRenderer.ShouldWindowClose()) {
        Vu::UpdateDeltaTime();

        //Pre-Render Begins
        spinningSystem.run();
        flyCameraSystem.run();

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
