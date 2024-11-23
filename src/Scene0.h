#pragma once

#include <filesystem>

#include "flecs.h"
#include "glm/gtx/string_cast.hpp"

#include "Camera.h"
#include "Components.h"
#include "VuMesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuAssetLoader.h"
#include "VuGlobalSetManager.h"
#include "VuRenderer.h"

using namespace Vu;

struct Scene0 {

    void Run() {

        VuRenderer vuRenderer;
        vuRenderer.Init();
        ctx::vuRenderer = &vuRenderer;
        VuMesh mesh{};

        auto helmetPath = "D:\\Dev\\Vulkan\\glTF-Sample-Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf";
        auto gnomePath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";

        VuShader shader;
        shader.CreateShader(
            "assets/shaders/vert.spv",
            "assets/shaders/frag.spv",
            vuRenderer.swapChain.renderPass.renderPass
        );
        uint32 mat0 = shader.CreateMaterial();

        PBRMaterialData* data = shader.materials[mat0].pbrMaterialData;
        VuAssetLoader::LoadGltf(gnomePath, mesh, *data);


        // VuTexture floorColorTex;
        // floorColorTex.alloc("assets/textures/cat.png", VK_FORMAT_R8G8B8A8_SRGB);
        //
        // VuTexture floorNormalTex;
        // floorNormalTex.alloc("assets/textures/cat_n.png", VK_FORMAT_R8G8B8A8_UNORM);
        //
        //
        // auto t0 = vuRenderer.globalSetManager.registerTexture(floorColorTex);
        // auto t1 = vuRenderer.globalSetManager.registerTexture(floorNormalTex);
        //
        // shader.materials[monkeMat0].pbrMaterialData->texture0 = t0;
        // shader.materials[monkeMat0].pbrMaterialData->texture1 = t1;

        // uint32 monkeMat1 = shader.createMaterial(&floorTex);


        flecs::world world;

        //Add Systems
        auto spinningSystem = AddSpinningSystem(world);
        auto flyCameraSystem = AddFlyCameraSystem(world);
        auto drawMeshSystem = AddRenderingSystem(world);
        auto spinUI = AddSpinUISystem(world);
        auto trsUI = AddTransformUISystem(world);
        auto camUI = AddCameraUISystem(world);


        //Add Entities
        world.entity("Obj1").set(Transform{
            .Position = float3(0, 0, 0), .Rotation = glm::quat(glm::vec3{0, 0, 0}), .Scale = {1, 1, 1}
        }).set(MeshRenderer{&mesh, &shader, mat0}).set(Spinn{});


        world.entity("Cam").set(
            Transform(float3(0, 0, 3.5f), float3(0, 0, 0), float3(1, 1, 1))
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
                    // Create the main docking space
                    ImGui::DockSpaceOverViewport(0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);

                    ImGui::Begin("Values");
                    ImGui::Text(std::format("Frame Per Second: {0:.0f}", (1.0f / ctx::deltaAsSecond)).c_str());
                    ImGui::Text(std::format("Frame Time as miliSec: {0:.4}", ctx::deltaAsSecond * 1000).c_str());
                    spinUI.run();
                    camUI.run();
                    trsUI.run();
                    ImGui::End();
                    ImGui::Begin("Rendering");
                    bool b = ImGui::Button("RealoadShader", { 1, 1 });
                    if (b) {
                        vuRenderer.reloadShaders();
                    }
                    ImGui::End();
                    vuRenderer.EndImgui();
                }

                vuRenderer.EndFrame();
            }
        }

        //Mission complete
        vuRenderer.WaitIdle();

        VuGlobalSetManager::decreaseTextureRefCount(shader.materials[mat0].pbrMaterialData->texture0);
        VuGlobalSetManager::decreaseTextureRefCount(shader.materials[mat0].pbrMaterialData->texture1);
        shader.Dispose();
        //floorColorTex.Dispose();
        //floorNormalTex.Dispose();
        mesh.Dispose();
        vuRenderer.Dispose();
        //system("pause");
    }
};
