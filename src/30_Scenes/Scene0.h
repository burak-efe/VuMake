#pragma once

#include <filesystem>

#include "flecs.h"
#include "tracy/Tracy.hpp"
#include "imgui_internal.h"

#include "11_Config/VuCtx.h"
#include "../08_LangUtils/VuPools.h"
#include "14_VuMake/VuAssetLoader.h"
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
        //Path meshPath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";
        path meshPath = "assets/gltf/Cube.glb";


        path gpassVertPath = "assets/shaders/GPass_vert.slang";
        path gpassFragPath = "assets/shaders/GPass_frag.slang";

        path defVertPath = "assets/shaders/screenTri_vert.slang";
        path defFragPath = "assets/shaders/deferred_frag.slang";

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
            PoolHandle imagePoolHnd             = {.index = 1};
            PoolHandle samplerPoolHnd           = {.index = 2};
            PoolHandle bufferPoolHnd            = {.index = 3};
            PoolHandle shaderPoolHnd            = {.index = 4};
            PoolHandle materialPoolHnd          = {.index = 5};
            PoolHandle materialDataIndexPoolHnd = {.index = 6};

            VuShader a{};

            VuResourcePool<VuShader>   shaderPool(1024, {1});
            VuResourcePool<VuImage>    imagePool(1024, AllocatorHandle::Default());
            VuResourcePool<VuSampler>  samplerPool(1024, AllocatorHandle::Default());
            VuResourcePool<VuBuffer>   bufferPool(1024, AllocatorHandle::Default());
            VuResourcePool<VuMaterial> materialPool(1024, AllocatorHandle::Default());
            VuResourcePool<u32>        materialDataIndexPool(1024, AllocatorHandle::Default());


            VuPoolManager::registerPoolToGlobalArray(imagePoolHnd, &imagePool);
            VuPoolManager::registerPoolToGlobalArray(samplerPoolHnd, &samplerPool);
            VuPoolManager::registerPoolToGlobalArray(bufferPoolHnd, &bufferPool);
            VuPoolManager::registerPoolToGlobalArray(shaderPoolHnd, &shaderPool);
            VuPoolManager::registerPoolToGlobalArray(materialPoolHnd, &materialPool);
            VuPoolManager::registerPoolToGlobalArray(materialDataIndexPoolHnd, &materialDataIndexPool);


            VuRenderer vuRenderer{};

            vuRenderer.init(imagePoolHnd,
                            samplerPoolHnd,
                            bufferPoolHnd,
                            shaderPoolHnd,
                            materialPoolHnd,
                            materialDataIndexPoolHnd);
            VuDevice* device = &vuRenderer.vuDevice;
            ECS_VU_RENDERER  = &vuRenderer;

            VuMesh mesh{};
            mesh.init(device);
            VuAssetLoader::LoadGltf(vuRenderer.vuDevice, meshPath, mesh);

            VuHnd<VuShader> gPassShaderHnd = device->createShader(gpassVertPath,
                                                                  gpassFragPath,
                                                                  &vuRenderer.swapChain.gBufferPass);

            VuHnd<VuShader> deferShaderHnd = device->createShader(defVertPath,
                                                                  defFragPath,
                                                                  &vuRenderer.swapChain.lightningPass);

            //mesh mat
            VuHnd<u32>            basicMatDataIndexHnd = vuRenderer.vuDevice.createMaterialDataIndex();
            VuHnd<VuMaterial>     basicMatHnd          = vuRenderer.vuDevice.createMaterial({}, gPassShaderHnd, basicMatDataIndexHnd);
            std::span<byte, 64>   basicMatDataBlob     = vuRenderer.vuDevice.getMaterialData(basicMatDataIndexHnd);
            GPU_PBR_MaterialData* basicMatData         = reinterpret_cast<GPU_PBR_MaterialData*>(basicMatDataBlob.data());
            basicMatData->texture0                     = 0;
            basicMatData->texture1                     = 1;

            //def mat
            VuHnd<u32>            deferMatDataIndexHnd = vuRenderer.vuDevice.createMaterialDataIndex();
            VuHnd<VuMaterial>     deferMatHnd          = vuRenderer.vuDevice.createMaterial({}, deferShaderHnd, deferMatDataIndexHnd);
            std::span<byte, 64>   deferMatDataBlob     = vuRenderer.vuDevice.getMaterialData(deferMatDataIndexHnd);
            GPU_PBR_MaterialData* deferMatData         = reinterpret_cast<GPU_PBR_MaterialData*>(deferMatDataBlob.data());
            deferMatData->texture0                     = vuRenderer.swapChain.colorHnd.index;
            deferMatData->texture1                     = vuRenderer.swapChain.normalHnd.index;
            deferMatData->texture2                     = vuRenderer.swapChain.armHnd.index;
            deferMatData->texture3                     = vuRenderer.swapChain.depthStencilHnd.index;


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
                          .position = vec3(0.0f, 0.0f, 0.0f),
                          .rotation = quaternion::identity(),
                          .scale = vec3(1.0F, 1.0F, 1.0F)
                      })
                 .set(MeshRenderer{.mesh = &mesh, .materialHnd = basicMatHnd})
                 .set(Spinn{});

            world.entity("Cam")
                 .set(Transform(vec3(0.0f, 0.0f, 3.5f),
                                quaternion::identity(),
                                vec3(1, 1, 1))
                     ).set(Camera{});


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
                    gPassShaderHnd.getResource()->tryRecompile();
                    deferShaderHnd.getResource()->tryRecompile();

                    vuRenderer.beginFrame();
                    drawMeshSystem.run();

                    vuRenderer.beginLightningPass();

                    vuRenderer.bindMaterial(deferMatHnd);
                    auto dataIndex = deferMatHnd.getResource()->materialDataHnd.index;
                    vuRenderer.pushConstants({mat4x4(), dataIndex});
                    vkCmdDraw(vuRenderer.commandBuffers[vuRenderer.currentFrame], 3, 1, 0, 0);


                    //UI
                    // {
                    //     vuRenderer.beginImgui();
                    //
                    //     ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                    //     auto vPort = ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);
                    //
                    //     auto wRes = ImGui::Begin("Info");
                    //     ImGui::Text("Image Count: %u", vuRenderer.vuDevice.imagePool.getUsedSlotCount());
                    //     ImGui::Text("Sampler Count: %u", vuRenderer.vuDevice.samplerPool.getUsedSlotCount());
                    //     ImGui::Text("Buffer Count: %u", vuRenderer.vuDevice.bufferPool.getUsedSlotCount());
                    //     ImGui::End();
                    //
                    //     if (uiNeedBuild)
                    //     {
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

            vuRenderer.waitIdle();
            gPassShaderHnd.destroyHandle();
            basicMatHnd.destroyHandle();
            basicMatDataIndexHnd.destroyHandle();
            mesh.uninit();
            vuRenderer.uninit();
        }
    };
}
