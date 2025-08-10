#include "VuGraphicsPipeline.h"

#include <array> // for array
#include <stdint.h>
#include <utility>
#include <vector> // for vector

#include "02_OuterCore/VuCommon.h"
#include "VuDevice.h"

Vu::VuGraphicsPipeline::VuGraphicsPipeline() = default;
Vu::VuGraphicsPipeline::VuGraphicsPipeline(VuGraphicsPipeline&& other) noexcept :
    m_vuDevice(std::move(other.m_vuDevice)),
    m_pipeline(other.m_pipeline) {
  other.m_pipeline = VK_NULL_HANDLE;
}
Vu::VuGraphicsPipeline&
Vu::VuGraphicsPipeline::operator=(VuGraphicsPipeline&& other) noexcept {
  if (this != &other) {
    cleanup();
    m_vuDevice       = std::move(other.m_vuDevice);
    m_pipeline       = other.m_pipeline;
    other.m_pipeline = VK_NULL_HANDLE;
  }
  return *this;
}
Vu::VuGraphicsPipeline::~VuGraphicsPipeline() { cleanup(); }
void
Vu::VuGraphicsPipeline::cleanup() {
  if (m_pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(m_vuDevice->m_device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;
  }
  m_vuDevice.reset();
}
Vu::VuGraphicsPipeline::VuGraphicsPipeline(std::shared_ptr<VuDevice>                      vuDevice,
                                           const VkPipelineLayout&                        pipelineLayout,
                                           const VkShaderModule&                          vertShaderModule,
                                           const VkShaderModule&                          fragShaderModule,
                                           const VkRenderPass&                            renderPass,
                                           std::span<VkPipelineColorBlendAttachmentState> colorBlends) :
    m_vuDevice(vuDevice) {
  VkPipelineShaderStageCreateInfo vertShaderStageInfo {};
  vertShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  vertShaderStageInfo.stage  = VK_SHADER_STAGE_VERTEX_BIT;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName  = "main";

  VkPipelineShaderStageCreateInfo fragShaderStageInfo {};
  fragShaderStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  fragShaderStageInfo.stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName  = "main";

  std::array shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

  VkPipelineVertexInputStateCreateInfo vertexInputInfo {};
  vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vertexInputInfo.vertexBindingDescriptionCount   = 0;
  vertexInputInfo.pVertexBindingDescriptions      = VK_NULL_HANDLE;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions    = VK_NULL_HANDLE;

  VkPipelineInputAssemblyStateCreateInfo inputAssembly {};
  inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  inputAssembly.primitiveRestartEnable = VK_FALSE;

  VkPipelineViewportStateCreateInfo viewportState {};
  viewportState.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  viewportState.viewportCount = 1;
  viewportState.scissorCount  = 1;

  VkPipelineRasterizationStateCreateInfo rasterizer {.sType =
                                                         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
  rasterizer.depthClampEnable        = VK_FALSE;
  rasterizer.rasterizerDiscardEnable = VK_FALSE;
  rasterizer.polygonMode             = VK_POLYGON_MODE_FILL;
  rasterizer.cullMode                = VK_CULL_MODE_BACK_BIT;
  rasterizer.frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rasterizer.depthBiasEnable         = VK_FALSE;
  rasterizer.lineWidth               = 1.0f;

  VkPipelineMultisampleStateCreateInfo multisampling {.sType =
                                                          VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
  multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  multisampling.sampleShadingEnable  = VK_FALSE;

  VkPipelineColorBlendStateCreateInfo colorBlending {.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
  colorBlending.logicOpEnable     = VK_FALSE;
  colorBlending.logicOp           = VK_LOGIC_OP_COPY;
  colorBlending.attachmentCount   = (uint32_t)colorBlends.size();
  colorBlending.pAttachments      = colorBlends.data();
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

  VkPipelineDynamicStateCreateInfo dynamicState {.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates    = dynamicStates.data();

  VkGraphicsPipelineCreateInfo pipelineInfo {.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
  pipelineInfo.stageCount          = shaderStages.size();
  pipelineInfo.pStages             = shaderStages.data();
  pipelineInfo.pVertexInputState   = &vertexInputInfo;
  pipelineInfo.pInputAssemblyState = &inputAssembly;
  pipelineInfo.pViewportState      = &viewportState;
  pipelineInfo.pRasterizationState = &rasterizer;
  pipelineInfo.pMultisampleState   = &multisampling;
  pipelineInfo.pColorBlendState    = &colorBlending;
  pipelineInfo.pDynamicState       = &dynamicState;
  pipelineInfo.layout              = pipelineLayout;
  pipelineInfo.renderPass          = renderPass;
  pipelineInfo.subpass             = 0;
  pipelineInfo.basePipelineHandle  = VK_NULL_HANDLE;

  VkPipelineDepthStencilStateCreateInfo depth = fillDepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
  pipelineInfo.pDepthStencilState             = &depth;
  VkResult gpRes =
      vkCreateGraphicsPipelines(vuDevice->m_device, VK_NULL_HANDLE, 1, &pipelineInfo, NO_ALLOC_CALLBACK, &m_pipeline);

  THROW_if_fail(gpRes);
}

VkPipelineDepthStencilStateCreateInfo
Vu::VuGraphicsPipeline::fillDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp) {
  VkPipelineDepthStencilStateCreateInfo info {.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
  info.depthTestEnable       = bDepthTest ? VK_TRUE : VK_FALSE;
  info.depthWriteEnable      = bDepthWrite ? VK_TRUE : VK_FALSE;
  info.depthCompareOp        = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
  info.depthBoundsTestEnable = VK_FALSE;
  info.minDepthBounds        = 0.0f;
  info.maxDepthBounds        = 1.0f;
  return info;
}
