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
#include "VuResourceManager.h"
#include "VuRenderer.h"

#include "tracy/Tracy.hpp"

using namespace Vu;

struct Scene0 {

    flecs::system spinningSystem;
    flecs::system flyCameraSystem;
    flecs::system drawMeshSystem;
    flecs::system spinUI;
    flecs::system trsUI;
    flecs::system camUI;

    VuRenderer vuRenderer;

    void UpdateLoop() {
        ZoneScoped;
        //Update Loop
        while (!vuRenderer.shouldWindowClose()) {
            ctx::PreUpdate();
            ctx::UpdateInput();

            //Pre-Render Begins
            spinningSystem.run();
            flyCameraSystem.run();

            //Rendering
            {
                vuRenderer.beginFrame();
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
                    bool b = ImGui::Button("RealoadShader", {1, 1});
                    if (b) {
                        vuRenderer.reloadShaders();
                    }
                    ImGui::End();
                    vuRenderer.EndImgui();
                }

                vuRenderer.endFrame();
            }
        }
    }

    void Run() {
        ZoneScoped;
        vuRenderer.init();
        ctx::vuRenderer = &vuRenderer;
        VuMesh mesh{};
        auto helmetPath = "D:\\Dev\\Vulkan\\glTF-Sample-Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf";
        auto gnomePath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";
        VuShader shader;
        shader.init(
            {
                "assets/shaders/vert.spv",
                "assets/shaders/frag.spv",
                vuRenderer.swapChain.renderPass.renderPass
            }
        );
        uint32 mat0 = shader.creatematerial();

        PBRMaterialData* data = shader.materials[mat0].pbrMaterialData;
        VuAssetLoader::LoadGltf(gnomePath, mesh, *data);
        //
        {
            ZoneScopedN("flecs");
            flecs::world world;
            //Add Systems
            spinningSystem = AddSpinningSystem(world);
            flyCameraSystem = AddFlyCameraSystem(world);
            drawMeshSystem = AddRenderingSystem(world);
            spinUI = AddSpinUISystem(world);
            trsUI = AddTransformUISystem(world);
            camUI = AddCameraUISystem(world);

            //Add Entities
            world.entity("Obj1").set(Transform{
                .Position = float3(0, 0, 0), .Rotation = glm::quat(glm::vec3{0, 0, 0}), .Scale = {1, 1, 1}
            }).set(MeshRenderer{&mesh, &shader, mat0}).set(Spinn{});

            world.entity("Cam").set(
                Transform(float3(0, 0, 3.5f), float3(0, 0, 0), float3(1, 1, 1))
            ).set(Camera{});

            UpdateLoop();

        }
        vuRenderer.waitIdle();
        VuResourceManager::decreaseTextureRefCount(shader.materials[mat0].pbrMaterialData->texture0);
        VuResourceManager::decreaseTextureRefCount(shader.materials[mat0].pbrMaterialData->texture1);
        shader.uninit();
        mesh.Dispose();
        vuRenderer.uninit();
    }
};
