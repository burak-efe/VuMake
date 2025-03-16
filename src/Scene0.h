#pragma once

#include <filesystem>

#include "flecs.h"
#include "tracy/Tracy.hpp"
#include "imgui_internal.h"

#include "Camera.h"
#include "Components.h"
#include "VuMesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuAssetLoader.h"
#include "VuRenderer.h"


namespace Vu
{
    struct Scene0
    {
    private:
        std::filesystem::path gnomePath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";

        VuHandle2<VuShader> shader{};
        VuRenderer          vuRenderer{};

        flecs::system spinningSystem;
        flecs::system flyCameraSystem;
        flecs::system drawMeshSystem;
        flecs::system spinUI;
        flecs::system trsUI;
        flecs::system camUI;

        bool uiNeedBuild = true;

        void UpdateLoop()
        {
            ZoneScoped;

            //Update Loop
            while (!vuRenderer.shouldWindowClose())
            {
                ctx::PreUpdate();
                ctx::UpdateInput();

                //Pre-Render Begins
                //auto runner0 = spinningSystem.run();
                auto runner1 = flyCameraSystem.run();

                //Rendering
                {
                    //shader.get()->tryRecompile();

                    vuRenderer.beginFrame();
                    drawMeshSystem.run();

                    // //UI
                    // {
                    //     vuRenderer.beginImgui();
                    //
                    //     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    //     auto vPort = ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);
                    //
                    //     auto wRes = ImGui::Begin("Info");
                    //     ImGui::Text("Texture Count: %u", VuPool<VuTexture>::getUsedSlotCount());
                    //     ImGui::Text("Sampler Count: %u", VuPool<VuSampler>::getUsedSlotCount());
                    //     ImGui::Text("Buffer Count: %u", VuPool<VuBuffer>::getUsedSlotCount());
                    //     ImGui::End();
                    //
                    //     if (uiNeedBuild) {
                    //         int32 dockspace_flags = 0;
                    //         uiNeedBuild           = false;
                    //
                    //         ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                    //         ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                    //         ImGui::DockBuilderSetNodeSize(dockspace_id, {
                    //                                           static_cast<float>(vuRenderer.swapChain.swapChainExtent.width),
                    //                                           static_cast<float>(vuRenderer.swapChain.swapChainExtent.height)
                    //                                       });
                    //
                    //         auto mainR = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
                    //         auto mainL = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.33f, nullptr, &dockspace_id);
                    //         ImGui::DockBuilderDockWindow("Info", mainL);
                    //
                    //         ImGui::DockBuilderFinish(dockspace_id);
                    //     }
                    //
                    //     vuRenderer.endImgui();
                    // }

                    vuRenderer.endFrame();
                }
            }
        }

    public:
        void Run()
        {
            ZoneScoped;
            //auto device   = VuDevice{};
            //ctx::vuDevice = &device;
            //std::cout << "Scene init" << std::endl;

            ctx::vuRenderer = &vuRenderer;
            ctx::vuDevice   = &vuRenderer.vuDevice;

            vuRenderer.init();

            VuMesh mesh{};

            shader          = vuRenderer.vuDevice.shaderPool.createHandle();
            auto* shaderPtr = vuRenderer.vuDevice.shaderPool.get(shader);

            shaderPtr->init(
                            {
                                "assets/shaders/vert.slang",
                                "assets/shaders/frag.slang",
                                vuRenderer.swapChain.renderPass.renderPass
                            }
                           );

            uint32 mat0 = shaderPtr->createMaterial();

            GPU_PBR_MaterialData* data = shaderPtr->materials[mat0].pbrMaterialData;

            VuAssetLoader::LoadGltf(vuRenderer.vuDevice, gnomePath, mesh, *data);


            flecs::world world;
            //Add Systems
            spinningSystem  = AddSpinningSystem(world);
            flyCameraSystem = AddFlyCameraSystem(world);
            drawMeshSystem  = AddRenderingSystem(world);
            spinUI          = AddSpinUISystem(world);
            trsUI           = AddTransformUISystem(world);
            camUI           = AddCameraUISystem(world);

            //Add Entities
            auto ent = world.entity("Obj1").set(Transform{
                                                    .position = float3(0, 0, 0), .rotation = quaternion::identity(),
                                                    .scale = float3(10.0F, 10.0F, 10.0F)
                                                }).set(MeshRenderer{&mesh, shader, mat0}).set(Spinn{});

            world.entity("Cam").set(Transform(float3(0.0f, 0.0f, 3.5f),
                                              quaternion::identity(),
                                              float3(1, 1, 1))
                                   ).set(Camera{});

            UpdateLoop();

            vuRenderer.waitIdle();
            //shader.destroyHandle();
            mesh.uninit();
            vuRenderer.uninit();
        }
    };
}
