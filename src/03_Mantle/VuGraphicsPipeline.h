#pragma once
#include "02_OuterCore/VuCommon.h"
namespace Vu {
struct VuDevice;
struct VuGraphicsPipeline {
  std::shared_ptr<VuDevice> m_vuDevice {nullptr};
  VkPipeline                m_pipeline {nullptr};

  SETUP_EXPECTED_WRAPPER(VuGraphicsPipeline,
                         (std::shared_ptr<VuDevice>                      vuDevice,
                          const VkPipelineLayout&                        pipelineLayout,
                          const VkShaderModule&                          vertShaderModule,
                          const VkShaderModule&                          fragShaderModule,
                          const VkRenderPass&                            renderPass,
                          std::span<VkPipelineColorBlendAttachmentState> colorBlends),
                         (vuDevice, pipelineLayout, vertShaderModule, fragShaderModule, renderPass, colorBlends))
public:
  VuGraphicsPipeline();

  VuGraphicsPipeline(const VuGraphicsPipeline&) = delete;

  VuGraphicsPipeline&
  operator=(const VuGraphicsPipeline&) = delete;

  VuGraphicsPipeline(VuGraphicsPipeline&& other) noexcept;

  VuGraphicsPipeline&
  operator=(VuGraphicsPipeline&& other) noexcept;

  ~VuGraphicsPipeline();

private:
  void
  cleanup();

  VuGraphicsPipeline(std::shared_ptr<VuDevice>                      vuDevice,
                     const VkPipelineLayout&                        pipelineLayout,
                     const VkShaderModule&                          vertShaderModule,
                     const VkShaderModule&                          fragShaderModule,
                     const VkRenderPass&                            renderPass,
                     std::span<VkPipelineColorBlendAttachmentState> colorBlends);

public:
  static VkPipelineDepthStencilStateCreateInfo
  fillDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
};
} // namespace Vu
