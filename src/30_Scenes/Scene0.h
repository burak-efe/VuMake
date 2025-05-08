#pragma once

#include <filesystem>

#include "tracy/Tracy.hpp"
#include "imgui_internal.h"

#include "11_Config/VuCtx.h"
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

    bool uiNeedBuild = true;

public:
    void Run()
    {
        VuShader a{};


        VuRenderer vuRenderer{};
        VuDevice*  device = &vuRenderer.vuDevice;
        ECS_VU_RENDERER   = &vuRenderer;

        VuMesh mesh{};
        mesh.init(device);
        VuAssetLoader::LoadGltf(vuRenderer.vuDevice, meshPath, mesh);

        std::shared_ptr<VuShader> gPassShaderHnd = device->createShader(gpassVertPath,
                                                                        gpassFragPath,
                                                                        &vuRenderer.swapChain.gBufferPass);

        std::shared_ptr<VuShader> deferShaderHnd = device->createShader(defVertPath,
                                                                        defFragPath,
                                                                        &vuRenderer.swapChain.lightningPass);

        //mesh mat
        std::shared_ptr<u32>        basicMatDataIndexHnd = vuRenderer.vuDevice.createMaterialDataIndex();
        std::shared_ptr<VuMaterial> basicMatHnd          = vuRenderer.vuDevice.
                                                             createMaterial({}, gPassShaderHnd,
                                                                            basicMatDataIndexHnd);
        std::span<byte, 64>   basicMatDataBlob = vuRenderer.vuDevice.getMaterialData(basicMatDataIndexHnd);
        GPU_PBR_MaterialData* basicMatData     = reinterpret_cast<GPU_PBR_MaterialData*>(basicMatDataBlob.data());
        basicMatData->texture0                 = 0;
        basicMatData->texture1                 = 1;

        //def mat
        std::shared_ptr<u32>        deferMatDataIndexHnd = vuRenderer.vuDevice.createMaterialDataIndex();
        std::shared_ptr<VuMaterial> deferMatHnd          = vuRenderer.vuDevice.
                                                             createMaterial({}, deferShaderHnd,
                                                                            deferMatDataIndexHnd);
        std::span<byte, 64>   deferMatDataBlob = vuRenderer.vuDevice.getMaterialData(deferMatDataIndexHnd);
        GPU_PBR_MaterialData* deferMatData     = reinterpret_cast<GPU_PBR_MaterialData*>(deferMatDataBlob.data());
        deferMatData->texture0                 = vuRenderer.swapChain.colorHnd.get()->bindlessIndex;
        deferMatData->texture1                 = vuRenderer.swapChain.normalHnd.get()->bindlessIndex;
        deferMatData->texture2                 = vuRenderer.swapChain.armHnd.get()->bindlessIndex;
        deferMatData->texture3                 = vuRenderer.swapChain.depthStencilHnd.get()->bindlessIndex;


        auto obj0Trs = Transform{
                .position = vec3(0.0f, 0.0f, 0.0f),
                .rotation = quaternion::identity(),
                .scale = vec3(1.0F, 1.0F, 1.0F)
        };

        auto obj1MeshRenderer = MeshRenderer{.mesh = &mesh, .materialHnd = basicMatHnd};
        auto obj1Spinn        = Spinn{};


        auto camTrs = Transform(vec3(0.0f, 0.0f, 3.5f),
                                quaternion::identity(),
                                vec3(1, 1, 1));
        auto cam = Camera{};


        //Update Loop
        while (!vuRenderer.shouldWindowClose())
        {
            ctx::PreUpdate();
            ctx::UpdateInput();

            //Pre-Render Begins
            cameraFlySystem(camTrs, cam);

            //Rendering
            {
                gPassShaderHnd->tryRecompile();
                deferShaderHnd->tryRecompile();

                vuRenderer.beginFrame();

                //user render commands begin
                drawMesh(obj0Trs, obj1MeshRenderer);
                //user render commands end

                vuRenderer.beginLightningPass();
                vuRenderer.bindMaterial(deferMatHnd);
                u32 dataIndex = *deferMatHnd.get()->materialDataHnd.get();
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
        vuRenderer.uninit();
    }
};
}
