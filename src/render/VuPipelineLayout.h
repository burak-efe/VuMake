#pragma once

#include <span>

#include "Common.h"

namespace Vu {
    namespace Initializers {
        inline void createPipelineLayout(const std::span<VkDescriptorSetLayout>& descriptorSetLayouts,
                                         const uint32 pushConstantSizeAsByte,
                                         VkPipelineLayout& outPipelineLayout) {
            ZoneScoped;

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

            VkCheck(vkCreatePipelineLayout(ctx::device, &pipelineLayoutInfo, nullptr, &outPipelineLayout));
        }

    }
}
