#include "VuGraphicsPipeline.h"

#include <cstdint>                 // for uint32_t
#include <array>                    // for array
#include <vector>                   // for vector

#include "VuCommon.h"
#include "08_LangUtils/TypeDefs.h"  // for u32
#include "10_Core/Common.h"       // for vk::Check

void Vu::VuGraphicsPipeline::initGraphicsPipeline(
    const vk::Device         device,
    const vk::PipelineLayout pipelineLayout,
    const vk::ShaderModule   vertShaderModule,
    const vk::ShaderModule   fragShaderModule,
    const vk::RenderPass     renderPass,
    std::span<vk::PipelineColorBlendAttachmentState> colorBlends
    )
{
    this->device = device;
    vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
    };

    vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
    };

    auto shaderStages = std::array{vertShaderStageInfo, fragShaderStageInfo};

    vk::PipelineVertexInputStateCreateInfo vertexInputInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 0,
        .pVertexBindingDescriptions = nullptr,
        .vertexAttributeDescriptionCount = 0,
        .pVertexAttributeDescriptions = nullptr,
    };

    vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    vk::PipelineViewportStateCreateInfo viewportState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1,
    };

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.0f,
    };

    vk::PipelineMultisampleStateCreateInfo multisampling{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
    };

    vk::PipelineColorBlendStateCreateInfo colorBlending{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = static_cast<uint32_t>(colorBlends.size()),
        .pAttachments = colorBlends.data(),
        .blendConstants = {0, 0, 0, 0},
    };

    //dynamic state
    std::vector<vk::DynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    vk::PipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<u32>(dynamicStates.size());
    dynamicState.pDynamicStates    = dynamicStates.data();

    vk::GraphicsPipelineCreateInfo pipelineInfo{
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

    vk::PipelineDepthStencilStateCreateInfo depth = fillDepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
    pipelineInfo.pDepthStencilState             = &depth;
    vk::Check(vkCreateGraphicsPipelines(device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
}

void Vu::VuGraphicsPipeline::uninit() const
{
    vkDestroyPipeline(device, pipeline, nullptr);
}

vk::PipelineDepthStencilStateCreateInfo Vu::VuGraphicsPipeline::fillDepthStencilCreateInfo(bool        bDepthTest, bool bDepthWrite,
                                                                                         vk::CompareOp compareOp)
{
    vk::PipelineDepthStencilStateCreateInfo info = {};
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
