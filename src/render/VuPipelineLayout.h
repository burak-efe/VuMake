#pragma once

#include <span>

#include "Common.h"
#include "VuUtils.h"
namespace Vu::Init {

    inline void CreatePipelineLayout(
        const std::span<VkDescriptorSetLayout>& descriptorSetLayouts,
        uint32 pushConstantSizeAsByte,
        VkPipelineLayout& pipelineLayout) {

        //push constants
        VkPushConstantRange push_constant;
        push_constant.offset = 0;
        push_constant.size = pushConstantSizeAsByte;
        push_constant.stageFlags = VK_SHADER_STAGE_ALL;


        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &push_constant;

        VK_CHECK(vkCreatePipelineLayout(Vu::Device, &pipelineLayoutInfo, nullptr, &pipelineLayout));
    }
}
