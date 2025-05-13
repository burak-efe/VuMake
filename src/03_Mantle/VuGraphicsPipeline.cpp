#include "VuGraphicsPipeline.h"

#include <array>  // for array
#include <vector> // for vector

#include "VuCommon.h"

Vu::VuGraphicsPipeline::VuGraphicsPipeline(const vk::raii::Device&                          device,
                                           const vk::raii::PipelineLayout&                  pipelineLayout,
                                           const vk::raii::ShaderModule&                    vertShaderModule,
                                           const vk::raii::ShaderModule&                    fragShaderModule,
                                           const vk::raii::RenderPass&                      renderPass,
                                           std::span<vk::PipelineColorBlendAttachmentState> colorBlends) {
  vk::PipelineShaderStageCreateInfo vertShaderStageInfo;
  vertShaderStageInfo.stage  = vk::ShaderStageFlagBits::eVertex;
  vertShaderStageInfo.module = vertShaderModule;
  vertShaderStageInfo.pName  = "main";

  vk::PipelineShaderStageCreateInfo fragShaderStageInfo;
  fragShaderStageInfo.stage  = vk::ShaderStageFlagBits::eFragment;
  fragShaderStageInfo.module = fragShaderModule;
  fragShaderStageInfo.pName  = "main";

  std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages = {vertShaderStageInfo, fragShaderStageInfo};

  vk::PipelineVertexInputStateCreateInfo vertexInputInfo;
  vertexInputInfo.vertexBindingDescriptionCount   = 0;
  vertexInputInfo.pVertexBindingDescriptions      = nullptr;
  vertexInputInfo.vertexAttributeDescriptionCount = 0;
  vertexInputInfo.pVertexAttributeDescriptions    = nullptr;

  vk::PipelineInputAssemblyStateCreateInfo inputAssembly;
  inputAssembly.topology               = vk::PrimitiveTopology::eTriangleList;
  inputAssembly.primitiveRestartEnable = vk::False;

  vk::PipelineViewportStateCreateInfo viewportState;
  viewportState.viewportCount = 1;
  viewportState.scissorCount  = 1;

  vk::PipelineRasterizationStateCreateInfo rasterizer;
  rasterizer.depthClampEnable        = vk::False;
  rasterizer.rasterizerDiscardEnable = vk::False;
  rasterizer.polygonMode             = vk::PolygonMode::eFill;
  rasterizer.cullMode                = vk::CullModeFlagBits::eBack;
  rasterizer.frontFace               = vk::FrontFace::eCounterClockwise;
  rasterizer.depthBiasEnable         = vk::False;
  rasterizer.lineWidth               = 1.0f;

  vk::PipelineMultisampleStateCreateInfo multisampling;
  multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
  multisampling.sampleShadingEnable  = vk::False;

  vk::PipelineColorBlendStateCreateInfo colorBlending;
  colorBlending.logicOpEnable     = vk::False;
  colorBlending.logicOp           = vk::LogicOp::eCopy;
  colorBlending.attachmentCount   = static_cast<uint32_t>(colorBlends.size());
  colorBlending.pAttachments      = colorBlends.data();
  colorBlending.blendConstants[0] = 0.0f;
  colorBlending.blendConstants[1] = 0.0f;
  colorBlending.blendConstants[2] = 0.0f;
  colorBlending.blendConstants[3] = 0.0f;

  std::vector<vk::DynamicState> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

  vk::PipelineDynamicStateCreateInfo dynamicState;
  dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
  dynamicState.pDynamicStates    = dynamicStates.data();

  vk::GraphicsPipelineCreateInfo pipelineInfo;
  pipelineInfo.stageCount          = static_cast<uint32_t>(shaderStages.size());
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

  vk::PipelineDepthStencilStateCreateInfo depth = fillDepthStencilCreateInfo(true, true, vk::CompareOp::eLessOrEqual);
  pipelineInfo.pDepthStencilState               = &depth;
  auto piplineOrErr                             = device.createGraphicsPipeline(nullptr, pipelineInfo);

  // todo
  throw_if_unexpected(piplineOrErr);
  pipeline = std::move(piplineOrErr.value());
}

vk::PipelineDepthStencilStateCreateInfo
Vu::VuGraphicsPipeline::fillDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, vk::CompareOp compareOp) {
  vk::PipelineDepthStencilStateCreateInfo info = {};
  info.depthTestEnable                         = bDepthTest ? vk::True : vk::False;
  info.depthWriteEnable                        = bDepthWrite ? vk::True : vk::False;
  info.depthCompareOp                          = bDepthTest ? compareOp : vk::CompareOp::eAlways;
  info.depthBoundsTestEnable                   = vk::False;
  info.minDepthBounds                          = 0.0f;
  info.maxDepthBounds                          = 1.0f;
  return info;
}
