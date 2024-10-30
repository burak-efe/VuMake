#pragma once

#include <filesystem>

#include "flecs.h"
#include "glm/gtx/string_cast.hpp"

#include "Camera.h"
#include "Components.h"
#include "VuMesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuRenderer.h"

using namespace Vu;

struct MonkeScene {

    void Run() {

        VuRenderer vuRenderer;
        vuRenderer.Init();
        ctx::vuRenderer = &vuRenderer;
        VuMesh monke("assets/meshes/monka500k.glb");

        VuShader shader;
        shader.InitShader("assets/shaders/vert.spv", "assets/shaders/frag.spv", vuRenderer.swapChain.renderPass.renderPass);
        uint32 monkeMat0 = shader.CreateMaterial();


        //vuRenderer.writeTexture();


        // VuTexture floorTex;
        // floorTex.Alloc("shaders/textures/floor0.png");
        // uint32 monkeMat1 = shader.createMaterial(&floorTex);


        flecs::world world;
        //world.set<VuRenderer*>(&vuRenderer);

        //Add Systems
        auto spinningSystem = AddSpinningSystem(world);
        auto flyCameraSystem = AddFlyCameraSystem(world);
        auto drawMeshSystem = AddRenderingSystem(world);
        auto spinUI = AddSpinUISystem(world);
        auto trsUI = AddTransformUISystem(world);
        auto camUI = AddCameraUISystem(world);


        //Add Entities
        world.entity("Monke1").set(Transform{
            .Position = float3(0, 0, 0)
        }).set(MeshRenderer{&monke, &shader, monkeMat0}).set(Spinn{});

        world.entity("Monke2").set(Transform{
            .Position = float3(4, 0, 0)
        }).set(MeshRenderer{&monke, &shader, monkeMat0}).set(Spinn{});

        world.entity("Cam").set(
            Transform(float3(0, 0, 4.0f), float3(0, 0, 0), float3(1, 1, 1))
        ).set(Camera{});


        //Update Loop
        while (!vuRenderer.ShouldWindowClose()) {
            ctx::PreUpdate();
            ctx::UpdateInput();

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
                    ImGui::Text(std::format("Frame Per Second: {0:.0f}", (1.0f / ctx::deltaAsSecond)).c_str());
                    ImGui::Text(std::format("Frame Time as miliSec: {0:.4}", ctx::deltaAsSecond * 1000).c_str());
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
        shader.Dispose();
        monke.Dispose();
        vuRenderer.Dispose();
        //system("pause");
    }
};
