#pragma once

#include "../02_OuterCore/VuCommon.h"

namespace Vu {
struct VuRenderPass {
  vk::raii::RenderPass                               renderPass                 = {nullptr};
  std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachmentStates = {};

  void
  initAsGBufferPass(const vk::raii::Device& device,
                    const vk::Format        colorFormat,
                    const vk::Format        normalFormat,
                    const vk::Format        aoRoughMetalFormat,
                    const vk::Format        worldPosFormat,
                    const vk::Format        depthStencilFormat);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void
  initAsLightningPass(const vk::raii::Device& device, vk::Format colorFormat);
};
} // namespace Vu
