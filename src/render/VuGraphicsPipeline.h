#pragma once
#include <span>

#include "Common.h"
#include "Vu.h"
#include "VuDepthStencil.h"


struct VuPipelineLayout {
    VkPipelineLayout pipelineLayout;

    void Dispose() const {
        vkDestroyPipelineLayout(Vu::Device, pipelineLayout, nullptr);
    }

    void CreatePipelineLayout(const std::span<VkDescriptorSetLayout>& descriptorSetLayouts, uint32 pushConstantSizeAsByte = 64) {

        //push constants
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = pushConstantSizeAsByte;
        push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;

        VK_CHECK(vkCreatePipelineLayout(Vu::Device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    }
};

struct VuGraphicsPipeline {
    VkPipeline Pipeline;

    void CreateGraphicsPipeline(
        const VkPipelineLayout& pipelineLayout,
        const VkShaderModule& vertShaderModule,
        const VkShaderModule& fragShaderModule,
        const std::span<VkVertexInputBindingDescription>& bindingDescriptions,
        const std::span<VkVertexInputAttributeDescription>& attributeDescriptions,
        const VkRenderPass& renderPass) {

        //assert(descriptorSetLayout != VK_NULL_HANDLE);
        assert(renderPass != VK_NULL_HANDLE);


        //sahder stages
        //auto vertShaderCode = Vu::ReadFile(vertexPath);
        //auto fragShaderCode = Vu::ReadFile(fragPath);

        //VkShaderModule vertShaderModule = Vu::CreateShaderModule(vertShaderCode);
        //VkShaderModule fragShaderModule = Vu::CreateShaderModule(fragShaderCode);

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

        auto shaderStages = array{vertShaderStageInfo, fragShaderStageInfo};

        //vertex bindings
        // auto bindingDescriptions = Mesh::getBindingDescription();
        // auto attributeDescriptions = Mesh::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount = static_cast<uint32>(bindingDescriptions.size()),
            .pVertexBindingDescriptions = bindingDescriptions.data(),
            .vertexAttributeDescriptionCount = static_cast<uint32>(attributeDescriptions.size()),
            .pVertexAttributeDescriptions = attributeDescriptions.data(),
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
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();


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

        auto depth = VuDepthStencil::CreateDepthStencilCreateInfo(true, true, VK_COMPARE_OP_LESS_OR_EQUAL);
        pipelineInfo.pDepthStencilState = &depth;

        VK_CHECK(vkCreateGraphicsPipelines(Vu::Device,VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &Pipeline));


    }

    void Dispose() const {
        vkDestroyPipeline(Vu::Device, Pipeline, nullptr);
    }
};
