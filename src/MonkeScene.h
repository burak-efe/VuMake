#pragma once

#include <filesystem>

#include "flecs.h"
#include "glm/gtx/string_cast.hpp"

#include "Camera.h"
#include "Components.h"
#include "Mesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuRenderer.h"

struct MonkeScene {

    void Run() {

        VuRenderer vuRenderer;
        vuRenderer.Init();
        Vu::Renderer = &vuRenderer;
        Mesh monke("shaders/monka.gltf");

        flecs::world world;
        world.set<VuRenderer>(vuRenderer);

        //Add Systems
        auto spinningSystem = AddSpinningSystem(world);
        auto flyCameraSystem = AddFlyCameraSystem(world);
        auto drawMeshSystem = AddRenderingSystem(world);
        auto spinUI = AddSpinUISystem(world);
       auto trsUI = AddTransformUISystem(world);
        auto camUI = AddCameraUISystem(world);


        //Add Entities
        world.entity("Monke2").set(Transform{
            .Position = float3(0, 0, 0)
        }).set(MeshRenderer{&monke}).set(Spinn{});

        world.entity("Cam").set(
            Transform(float3(0, 0, 4.0f), float3(0, 0, 0), float3(1, 1, 1))
        ).set(Camera{});


        //Update Loop
        while (!vuRenderer.ShouldWindowClose()) {
            Vu::PreUpdate();
            Vu::UpdateInput();

            //Pre-Render Begins
            spinningSystem.run();
            flyCameraSystem.run();

            //Rendering
            {
                vuRenderer.BeginFrame();
                drawMeshSystem.run();

                //UI
                {
                    vuRenderer.BeginImgui();
                    ImGui::Text(std::format("Frame Per Second: {0:.0f}", (1.0f / Vu::DeltaAsSecond)).c_str());
                    ImGui::Text(std::format("Frame Time as miliSec: {0:.4}", Vu::DeltaAsSecond * 1000).c_str());
                    spinUI.run();
                    camUI.run();
                    trsUI.run();
                    vuRenderer.EndImgui();
                }

                vuRenderer.EndFrame();
            }
        }

        //Mission complete
        vuRenderer.WaitIdle();
        monke.Dispose();
        vuRenderer.Dispose();
        system("pause");
    }
};
