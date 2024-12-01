#pragma once

#include <filesystem>

#include "flecs.h"
#include "glm/gtx/string_cast.hpp"

#include "Camera.h"
#include "Components.h"
#include "imgui_internal.h"
#include "VuMesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuAssetLoader.h"
#include "VuResourceManager.h"
#include "VuRenderer.h"

#include "tracy/Tracy.hpp"
#include "async++.h"

using namespace Vu;

struct Scene0 {

    std::filesystem::path helmetPath = "D:\\Dev\\Vulkan\\glTF-Sample-Assets\\Models\\DamagedHelmet\\glTF\\DamagedHelmet.gltf";
    std::filesystem::path gnomePath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";

    VuShader shader;

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
                shader.tryRecompile();

                vuRenderer.beginFrame();
                drawMeshSystem.run();

                //UI
                {
                    vuRenderer.BeginImgui();

                    uint32 dockspace_flags = 0;
                    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    //ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
                    ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);

                    ImGui::Begin("Entites");
                    ImGui::End();

                    ImGui::Begin("Shaders");
                    ImGui::End();

                    ImGui::Begin("Meshes");
                    ImGui::End();

                    ImGui::Begin("Textures");
                    ImGui::End();

                    static auto first_time = true;
                    if (first_time) {
                        first_time = false;

                        ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                        ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                        ImGui::DockBuilderSetNodeSize(dockspace_id, {
                                                          static_cast<float>(vuRenderer.swapChain.swapChainExtent.width),
                                                          static_cast<float>(vuRenderer.swapChain.swapChainExtent.height)
                                                      });

                        auto mainR = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
                        auto mainL = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.33f, nullptr, &dockspace_id);
                        ImGui::DockBuilderDockWindow("Entites", mainR);
                        ImGui::DockBuilderDockWindow("Meshes", mainL);
                        ImGui::DockBuilderDockWindow("Shaders", mainL);

                        ImGui::DockBuilderFinish(dockspace_id);
                    }


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


        shader.init(
            {
                "assets\\shaders\\vert.slang",
                "assets\\shaders\\frag.slang",
                vuRenderer.swapChain.renderPass.renderPass
            }
        );

        uint32 mat0 = shader.createMaterial();

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
            auto ent = world.entity("Obj1").set(Transform{
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
