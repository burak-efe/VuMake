#pragma once

#include <filesystem>

#include "02_OuterCore/Common.h"
#include "03_Mantle/VuImage.h"
#include "04_Crust/VuAssetLoader.h"
#include "04_Crust/VuMaterial.h"
#include "04_Crust/VuMesh.h"
#include "04_Crust/VuRenderer.h"
#include "04_Crust/VuShader.h"
#include "11_Components/Camera.h"
#include "11_Components/Components.h"
#include "11_Components/Transform.h"
#include "12_Systems/Systems.h"
#include "imgui.h"
#include "imgui_internal.h"

namespace Vu {
struct VuShader;

struct Scene_GLTF_Load {
private:
  path gltfPath     = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";
  path gltfCubePath = "assets/gltf/Cube.glb";

  path pbrVertPath = "assets/shaders/object/deferred/pbr_deferred_vert.slang";
  path pbrFragPath = "assets/shaders/object/deferred/pbr_deferred_frag.slang";

  path defVertPath = "assets/shaders/engine/screen_space_triangle_vert.slang";
  path defFragPath = "assets/shaders/engine/deferred_render_space/deferred_lightning_pass_frag.slang";

  bool uiNeedBuild = true;

public:
  void
  run() const {
    constexpr Vu::VuRendererCreateInfo info {};
    std::shared_ptr<VuRenderer>        vuRenderer = std::make_shared<VuRenderer>(info);

    // create a mesh asset
    VuMesh mesh {};
    VuAssetLoader::loadGLTF(*vuRenderer, gltfPath, mesh);

    // basic shader
    std::shared_ptr<VuShader> basicShader = std::make_shared<VuShader>(move_or_THROW(
        VuShader::make(vuRenderer, vuRenderer->m_deferredRenderSpace.m_gBufferPass, pbrVertPath, pbrFragPath)));

    // deffered lpas shder
    std::shared_ptr<VuShader> lPassShader = std::make_shared<VuShader>(move_or_THROW(
        VuShader::make(vuRenderer, vuRenderer->m_deferredRenderSpace.m_lightningPass, defVertPath, defFragPath)));

    MaterialSettings defaultMaterialSettings {};

    // creating basic material for object
    std::shared_ptr<VuMaterialDataHandle> basicMatDataHnd = vuRenderer->createMaterialDataIndex();
    std::shared_ptr<VuMaterial>           basicMaterial =
        std::make_shared<VuMaterial>(defaultMaterialSettings, basicShader, basicMatDataHnd);

    // write material data
    auto*   basicMatData   = vuRenderer->getMaterialDataPointerAs<MatData_PbrDeferred>(*basicMatDataHnd);
    VuImage colorMapOrErr  = move_or_THROW(VuAssetLoader::loadMapFromGLTF(*vuRenderer, gltfPath, MapType::baseColor));
    VuImage normalMapOrErr = move_or_THROW(VuAssetLoader::loadMapFromGLTF(*vuRenderer, gltfPath, MapType::normal));
    VuImage arm_MapOrErr =
        move_or_THROW(VuAssetLoader::loadMapFromGLTF(*vuRenderer, gltfPath, MapType::ao_rough_metal));

    vuRenderer->registerToBindless(colorMapOrErr);
    vuRenderer->registerToBindless(normalMapOrErr);
    vuRenderer->registerToBindless(arm_MapOrErr);

    basicMatData->colorTexture        = colorMapOrErr.m_bindlessIndex.value_or_THROW();
    basicMatData->normalTexture       = normalMapOrErr.m_bindlessIndex.value_or_THROW();
    basicMatData->aoRoughMetalTexture = arm_MapOrErr.m_bindlessIndex.value_or_THROW();

    // lightning pass material
    std::shared_ptr<VuMaterialDataHandle> lPassMatDataHandle = vuRenderer->createMaterialDataIndex();
    std::shared_ptr<VuMaterial>           lPassMaterial =
        std::make_shared<VuMaterial>(defaultMaterialSettings, lPassShader, lPassMatDataHandle);

    auto* lPassMatData = vuRenderer->getMaterialDataPointerAs<MatData_PbrDeferred>(*lPassMatDataHandle);
    *lPassMatData      = vuRenderer->m_deferredRenderSpace.m_lightningPassMaterialData;

    auto obj0Trs = Transform {
        .m_position = float3(0.0f, 0.0f, 0.0f), .rotation = quaternion::identity(), .scale = float3(10.0F, 10.0F, 10.0F)};

    auto obj1MeshRenderer = MeshRenderer {.mesh = &mesh, .materialHnd = basicMaterial};
    auto obj1Spinn        = Spinn {};

    auto camTrs = Transform(float3(0.0f, 0.0f, 3.5f), quaternion::identity(), float3(1, 1, 1));
    auto cam    = Camera {};

    // Update Loop
    while (!vuRenderer->shouldWindowClose()) {
      vuRenderer->preUpdate();
      vuRenderer->pollUserInput();

      auto& pl0 = vuRenderer->m_frameConst.pointLights[0];
      auto& pl1 = vuRenderer->m_frameConst.pointLights[1];

      pl0.range = 100;
      pl1.range = 100;

      pl0.color = float3(1.0f, 0.0f, 0.0f);
      pl1.color = float3(1.0f, 1.0f, 1.0f);

      pl0.intensity = 1000.0f;
      pl1.intensity = 1000.0f;

      pl0.position = float3(5.0f, 10.0f, 0.0f);
      pl1.position = float3(-5.0f, 10.0f, 0.0f);

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
        VuMaterialDataHandle dataIndex = *lPassMaterial->m_materialDataHnd;
        vuRenderer->pushConstants({float4x4(), dataIndex});
        vkCmdDraw(vuRenderer->m_commandBuffers[vuRenderer->m_currentFrame], 3, 1, 0, 0);

        // UI
        {
          vuRenderer->beginImgui();

          ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
          auto    vPort =
              ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);

          auto wRes = ImGui::Begin("Info");
          // ImGui::Text("Image Count: %u", vuRenderer.vuDevice.imagePool.getUsedSlotCount());
          // ImGui::Text("Sampler Count: %u", vuRenderer.vuDevice.samplerPool.getUsedSlotCount());
          // ImGui::Text("Buffer Count: %u", vuRenderer.vuDevice.bufferPool.getUsedSlotCount());
          ImGui::End();

          static bool uiNeedBuild = true;

          if (uiNeedBuild) {
            int32_t dockspace_flags = 0;
            uiNeedBuild             = false;

            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
            auto extend = vuRenderer->m_deferredRenderSpace.m_vuSwapChain.m_extend2D;
            ImGui::DockBuilderSetNodeSize(dockspace_id,
                                          {static_cast<float>(extend.width), static_cast<float>(extend.height)});

            auto mainR = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
            auto mainL = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.33f, nullptr, &dockspace_id);
            ImGui::DockBuilderDockWindow("Info", mainL);

            ImGui::DockBuilderFinish(dockspace_id);
          }

          vuRenderer->endImgui();
        }

        vuRenderer->endFrame();
      }
    }
    VkResult waitRes = vkDeviceWaitIdle(vuRenderer->m_vuDevice->m_device);

    THROW_if_fail(waitRes);
  }
};
} // namespace Vu
