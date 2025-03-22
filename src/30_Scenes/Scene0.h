#pragma once

#include <filesystem>

#include "flecs.h"
#include "tracy/Tracy.hpp"
#include "imgui_internal.h"

#include "11_Config/VuCtx.h"
#include "14_VuMake/VuAssetLoader.h"
#include "../12_VuMakeCore/VuPools.h"
#include "14_VuMake/VuRenderer.h"
#include "20_Components/Camera.h"
#include "20_Components/Components.h"
#include "20_Components/Transform.h"
#include "21_Systems/Systems.h"


namespace Vu
{
    struct VuShader;

    struct Scene0
    {
    private:
        Path gnomePath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";

        VuHnd<VuShader> shaderHnd;
        VuRenderer      vuRenderer{};

        flecs::system spinningSystem;
        flecs::system flyCameraSystem;
        flecs::system drawMeshSystem;
        flecs::system spinUI;
        flecs::system trsUI;
        flecs::system camUI;
        bool          uiNeedBuild = true;

    public:
        void Run()
        {
            ZoneScoped;
            Logger::SetLevel(LogLevel::Trace);
            vuRenderer.init();
            VuDevice* device = &vuRenderer.vuDevice;
            ECS_VU_RENDERER  = &vuRenderer;

            VuMesh mesh{};

            shaderHnd = device->createShader("assets/shaders/vert.slang",
                                             "assets/shaders/frag.slang",
                                             vuRenderer.swapChain.renderPass.renderPass);

            auto matDataIndexHnd = vuRenderer.vuDevice.createMaterialDataIndex();

            auto matHnd = vuRenderer.vuDevice.createMaterial({}, shaderHnd, matDataIndexHnd);

            GPU_PBR_MaterialData* data = vuRenderer.vuDevice.getMaterialData(matDataIndexHnd);

            VuAssetLoader::LoadGltf(vuRenderer.vuDevice, gnomePath, mesh, *data);

            data->texture0 = 0;
            data->texture1 = 1;

            flecs::world world;
            //Add Systems
            spinningSystem  = AddSpinningSystem(world);
            flyCameraSystem = AddFlyCameraSystem(world);
            drawMeshSystem  = AddRenderingSystem(world);
            spinUI          = AddSpinUISystem(world);
            trsUI           = AddTransformUISystem(world);
            camUI           = AddCameraUISystem(world);

            //Add Entities
            world.entity("Obj1")
                 .set(Transform{
                          .position = float3(0, 0, 0),
                          .rotation = quaternion::identity(),
                          .scale = float3(10.0F, 10.0F, 10.0F)
                      })
                 .set(MeshRenderer{.mesh = &mesh, .materialHnd = matHnd})
                 .set(Spinn{});

            world.entity("Cam")
                 .set(Transform(float3(0.0f, 0.0f, 3.5f),
                                quaternion::identity(),
                                float3(1, 1, 1))
                     ).set(Camera{});


            //Update Loop
            while (!vuRenderer.shouldWindowClose())
            {
                ctx::PreUpdate();
                ctx::UpdateInput();

                //Pre-Render Begins
                auto runner0 = spinningSystem.run();
                auto runner1 = flyCameraSystem.run();

                //Rendering
                {
                    vuRenderer.vuDevice.getShader(shaderHnd)->tryRecompile();

                    vuRenderer.beginFrame();
                    drawMeshSystem.run();

                    //UI
                    {
                        vuRenderer.beginImgui();

                        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                        auto vPort = ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);

                        auto wRes = ImGui::Begin("Info");
                        ImGui::Text("Image Count: %u", vuRenderer.vuDevice.imagePool.getUsedSlotCount());
                        ImGui::Text("Sampler Count: %u", vuRenderer.vuDevice.samplerPool.getUsedSlotCount());
                        ImGui::Text("Buffer Count: %u", vuRenderer.vuDevice.bufferPool.getUsedSlotCount());
                        ImGui::End();

                        if (uiNeedBuild)
                        {
                            int32 dockspace_flags = 0;
                            uiNeedBuild           = false;

                            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                            ImGui::DockBuilderSetNodeSize(dockspace_id, {
                                                              static_cast<float>(vuRenderer.swapChain.swapChainExtent.width),
                                                              static_cast<float>(vuRenderer.swapChain.swapChainExtent.height)
                                                          });

                            auto mainR = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
                            auto mainL = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.33f, nullptr, &dockspace_id);
                            ImGui::DockBuilderDockWindow("Info", mainL);

                            ImGui::DockBuilderFinish(dockspace_id);
                        }

                        vuRenderer.endImgui();
                    }

                    vuRenderer.endFrame();
                }
            }

            vuRenderer.waitIdle();
            device->destroyHandle(shaderHnd);
            device->destroyHandle(matHnd);
            device->destroyHandle(matDataIndexHnd);
            mesh.uninit();
            vuRenderer.uninit();
        }
    };
}
