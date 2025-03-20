#include "VuGraphicsPipeline.h"

#include "VuDevice.h"
#include "11_Config/VuCtx.h"

void Vu::VuGraphicsPipeline::initGraphicsPipeline(const VkPipelineLayout pipelineLayout, const VkShaderModule vertShaderModule,
                                                  const VkShaderModule fragShaderModule, const std::span<VkVertexInputBindingDescription> bindingDescriptions,
                                                  const std::span<VkVertexInputAttributeDescription> attributeDescriptions, const VkRenderPass renderPass)
{

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    auto shaderStages = std::array{vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,      //static_cast<uint32>(bindingDescriptions.size()),
        .pVertexBindingDescriptions = nullptr,   //bindingDescriptions.data(),
        .vertexAttributeDescriptionCount = 0,    // static_cast<uint32>(attributeDescriptions.size()),
        .pVertexAttributeDescriptions = nullptr, //attributeDescriptions.data(),
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkPipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_FRONT_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    VkPipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0, 0, 0, 0},
    };

    //dynamic state
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2,
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputInfo,
        .pInputAssemblyState = &inputAssembly,
        .pViewportState = &viewportState,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampling,
        .pColorBlendState = &colorBlending,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE
    };

    VkPipelineDepthStencilStateCreateInfo depth = fillDepthStencilCreateInfo(
                                                                             true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
    pipelineInfo.pDepthStencilState = &depth;

    VkCheck(vkCreateGraphicsPipelines(ctx::vuDevice->device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
}

void Vu::VuGraphicsPipeline::Dispose() const
{
    vkDestroyPipeline(ctx::vuDevice->device, pipeline, nullptr);
}

VkPipelineDepthStencilStateCreateInfo Vu::VuGraphicsPipeline::fillDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite,
    VkCompareOp compareOp)
{
    VkPipelineDepthStencilStateCreateInfo info = {};
    info.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    info.depthTestEnable                       = bDepthTest ? VK_TRUE : VK_FALSE;
    info.depthWriteEnable                      = bDepthWrite ? VK_TRUE : VK_FALSE;
    info.depthCompareOp                        = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
    info.depthBoundsTestEnable                 = VK_FALSE;
    info.minDepthBounds                        = 0.0f; // Optional
    info.maxDepthBounds                        = 1.0f; // Optional
    //info.stencilTestEnable = VK_TRUE;

    return info;
}
