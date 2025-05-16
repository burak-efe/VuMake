#pragma once

#include <filesystem>

#include "02_OuterCore/Common.h"
#include "04_Crust/VuAssetLoader.h"
#include "04_Crust/VuRenderer.h"
#include "11_Components/Camera.h"
#include "11_Components/Components.h"
#include "11_Components/Transform.h"
#include "12_Systems/Systems.h"

namespace Vu {
struct VuShader;

struct Scene0 {
private:
  // Path meshPath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";
  path meshPath = "assets/gltf/Cube.glb";

  path gpassVertPath = "assets/shaders/GPass_vert.slang";
  path gpassFragPath = "assets/shaders/GPass_frag.slang";

  path defVertPath = "assets/shaders/screenTri_vert.slang";
  path defFragPath = "assets/shaders/deferred_frag.slang";

  bool uiNeedBuild = true;

public:
  void
  Run() {
    VuShader a {};

    constexpr Vu::VuRendererCreateInfo info {};

    std::shared_ptr<VuRenderer> vuRenderer = std::make_shared<VuRenderer>(info);

    VuDevice& device = *vuRenderer->vuDevice;
    // ECS_VU_RENDERER  = &vuRenderer;

    VuMesh mesh {};
    // mesh.init(device);
    VuAssetLoader::LoadGltf(device, meshPath, mesh);

    // std::shared_ptr<VuShader> gPassShaderHnd =
    //     device->createShader(gpassVertPath, gpassFragPath, &vuRenderer.swapChain.gBufferPass);

    std::shared_ptr<VuShader> gPassShaderHnd = std::make_shared<VuShader>(move_or_throw(
        VuShader::make(vuRenderer, vuRenderer->deferredRenderSpace.gBufferPass, gpassVertPath, gpassFragPath)));

    // device.createShader(defVertPath, defFragPath, &vuRenderer.swapChain.lightningPass);
    std::shared_ptr<VuShader> deferShaderHnd = std::make_shared<VuShader>(move_or_throw(
        VuShader::make(vuRenderer, vuRenderer->deferredRenderSpace.lightningPass, defVertPath, defFragPath)));

    // mesh mat
    std::shared_ptr<VuMaterialDataHandle> basicMatDataIndexHnd = vuRenderer->createMaterialDataIndex();
    MaterialSettings                      matSettings {};
    std::shared_ptr<VuMaterial>           basicMatHnd =
        std::make_shared<VuMaterial>(matSettings, gPassShaderHnd, basicMatDataIndexHnd);
    std::span<byte, 64>   basicMatDataBlob = vuRenderer->getMaterialData(basicMatDataIndexHnd);
    GPU_PBR_MaterialData* basicMatData     = reinterpret_cast<GPU_PBR_MaterialData*>(basicMatDataBlob.data());
    basicMatData->texture0                 = 0;
    basicMatData->texture1                 = 1;

    // def mat
    std::shared_ptr<VuMaterialDataHandle> deferMatDataIndexHnd = vuRenderer->createMaterialDataIndex();
    //     vuRenderer->vuDevice->createMaterial({}, deferShaderHnd, deferMatDataIndexHnd);
    std::shared_ptr<VuMaterial> deferMatHnd =
        std::make_shared<VuMaterial>(matSettings, deferShaderHnd, deferMatDataIndexHnd);
    std::span<byte, 64>   deferMatDataBlob = vuRenderer->getMaterialData(deferMatDataIndexHnd);
    GPU_PBR_MaterialData* deferMatData     = reinterpret_cast<GPU_PBR_MaterialData*>(deferMatDataBlob.data());
    deferMatData->texture0                 = vuRenderer->deferredRenderSpace.colorHnd->bindlessIndex;
    deferMatData->texture1                 = vuRenderer->deferredRenderSpace.normalHnd->bindlessIndex;
    deferMatData->texture2                 = vuRenderer->deferredRenderSpace.armHnd->bindlessIndex;
    deferMatData->texture3                 = vuRenderer->deferredRenderSpace.depthStencilHnd->bindlessIndex;

    auto obj0Trs = Transform {
        .position = vec3(0.0f, 0.0f, 0.0f), .rotation = quaternion::identity(), .scale = vec3(1.0F, 1.0F, 1.0F)};

    auto obj1MeshRenderer = MeshRenderer {.mesh = &mesh, .materialHnd = basicMatHnd};
    auto obj1Spinn        = Spinn {};

    auto camTrs = Transform(vec3(0.0f, 0.0f, 3.5f), quaternion::identity(), vec3(1, 1, 1));
    auto cam    = Camera {};

    // Update Loop
    while (!vuRenderer->shouldWindowClose()) {
      vuRenderer->PreUpdate();
      vuRenderer->UpdateInput();

      // Pre-Render Begins
      cameraFlySystem(*vuRenderer, camTrs, cam);

      // Rendering
      {
        gPassShaderHnd->tryRecompile();
        deferShaderHnd->tryRecompile();

        vuRenderer->beginFrame();

        // user render commands begin
        drawMesh(*vuRenderer, obj0Trs, obj1MeshRenderer);
        // user render commands end

        vuRenderer->beginLightningPass();
        vuRenderer->bindMaterial(deferMatHnd);
        auto dataIndex = *deferMatHnd.get()->materialDataHnd.get();
        vuRenderer->pushConstants({mat4x4(), dataIndex});
        vuRenderer->commandBuffers[vuRenderer->currentFrame].draw(3, 1, 0, 0);
        // vkCmdDraw(vuRenderer.commandBuffers[vuRenderer.currentFrame], 3, 1, 0, 0);

        // UI
        //  {
        //      vuRenderer.beginImgui();
        //
        //      ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
        //      auto vPort = ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode,
        //      nullptr);
        //
        //      auto wRes = ImGui::Begin("Info");
        //      ImGui::Text("Image Count: %u", vuRenderer.vuDevice.imagePool.getUsedSlotCount());
        //      ImGui::Text("Sampler Count: %u", vuRenderer.vuDevice.samplerPool.getUsedSlotCount());
        //      ImGui::Text("Buffer Count: %u", vuRenderer.vuDevice.bufferPool.getUsedSlotCount());
        //      ImGui::End();
        //
        //      if (uiNeedBuild)
        //      {
        //          int32 dockspace_flags = 0;
        //          uiNeedBuild           = false;
        //
        //          ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
        //          ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
        //          ImGui::DockBuilderSetNodeSize(dockspace_id, {
        //                                            static_cast<float>(vuRenderer.swapChain.swapChainExtent.width),
        //                                            static_cast<float>(vuRenderer.swapChain.swapChainExtent.height)
        //                                        });
        //
        //          auto mainR = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr,
        //          &dockspace_id); auto mainL = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.33f,
        //          nullptr, &dockspace_id); ImGui::DockBuilderDockWindow("Info", mainL);
        //
        //          ImGui::DockBuilderFinish(dockspace_id);
        //      }
        //
        //      vuRenderer.endImgui();
        //  }

        vuRenderer->endFrame();
      }
    }

    vuRenderer->vuDevice->device.waitIdle();
    // vuRenderer.uninit();
  }
};
} // namespace Vu
