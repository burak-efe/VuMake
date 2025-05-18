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
  // path meshPath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";
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
    VuAssetLoader::LoadGltf(*vuRenderer, meshPath, mesh);

    std::shared_ptr<VuShader> basicShader = std::make_shared<VuShader>(move_or_throw(
        VuShader::make(vuRenderer, vuRenderer->deferredRenderSpace.gBufferPass, gpassVertPath, gpassFragPath)));

    std::shared_ptr<VuShader> lPassShader = std::make_shared<VuShader>(move_or_throw(
        VuShader::make(vuRenderer, vuRenderer->deferredRenderSpace.lightningPass, defVertPath, defFragPath)));

    MaterialSettings defaultMaterialSettings {};

    std::shared_ptr<VuMaterialDataHandle> basicMatDataIndexHnd = vuRenderer->createMaterialDataIndex();
    std::shared_ptr<VuMaterial>           basicMaterial =
        std::make_shared<VuMaterial>(defaultMaterialSettings, basicShader, basicMatDataIndexHnd);

    std::span<byte, 64> basicMatDataBlob = vuRenderer->getMaterialDataSpan(basicMatDataIndexHnd);
    auto*               basicMatData     = reinterpret_cast<GPU_PBR_MaterialData*>(basicMatDataBlob.data());
    basicMatData->texture0               = vuRenderer->defaultImage->bindlessIndex;
    basicMatData->texture1               = vuRenderer->defaultNormalImage->bindlessIndex;

    // lightning pass material
    std::shared_ptr<VuMaterialDataHandle> lPassMatDataHandle = vuRenderer->createMaterialDataIndex();
    std::shared_ptr<VuMaterial>           lPassMaterial =
        std::make_shared<VuMaterial>(defaultMaterialSettings, lPassShader, lPassMatDataHandle);

    std::span<byte, 64> lPassMatDataSpan = vuRenderer->getMaterialDataSpan(lPassMatDataHandle);
    auto*               lPassMatData     = reinterpret_cast<GPU_PBR_MaterialData*>(lPassMatDataSpan.data());

    std::memcpy(lPassMatData, &vuRenderer->deferredRenderSpace.lightningPassMaterialData, 64);

    auto obj0Trs = Transform {
        .position = vec3(0.0f, 0.0f, 0.0f), .rotation = quaternion::identity(), .scale = vec3(1.0F, 1.0F, 1.0F)};

    auto obj1MeshRenderer = MeshRenderer {.mesh = &mesh, .materialHnd = basicMaterial};
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
        basicShader->tryRecompile();
        lPassShader->tryRecompile();

        vuRenderer->beginFrame();

        // user render commands begin
        drawMesh(*vuRenderer, obj0Trs, obj1MeshRenderer);
        // user render commands end

        vuRenderer->beginLightningPass();
        vuRenderer->bindMaterial(lPassMaterial);
        VuMaterialDataHandle dataIndex = *lPassMaterial->materialDataHnd;
        vuRenderer->pushConstants({mat4x4(), dataIndex});
        std::span<byte, 64> span   = vuRenderer->getMaterialDataSpan(lPassMatDataHandle);
        auto                target = (GPU_PBR_MaterialData*)span.data();
        std::memcpy(target, &vuRenderer->deferredRenderSpace.lightningPassMaterialData, 64);
        vuRenderer->commandBuffers[vuRenderer->currentFrame].draw(3, 1, 0, 0);

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
