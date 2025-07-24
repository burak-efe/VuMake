#pragma once
#include "02_OuterCore/VuCommon.h"
namespace vk {
enum class CompareOp;
} // namespace vk

namespace Vu {
struct VuGraphicsPipeline {
  vk::raii::Pipeline pipeline = {nullptr};

  VuGraphicsPipeline() = default;

  VuGraphicsPipeline(const vk::raii::Device&                          device,
                     const vk::raii::PipelineLayout&                  pipelineLayout,
                     const vk::raii::ShaderModule&                    vertShaderModule,
                     const vk::raii::ShaderModule&                    fragShaderModule,
                     const vk::raii::RenderPass&                      renderPass,
                     std::span<vk::PipelineColorBlendAttachmentState> colorBlends);
  static vk::PipelineDepthStencilStateCreateInfo
  fillDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, vk::CompareOp compareOp);
};
} // namespace Vu
