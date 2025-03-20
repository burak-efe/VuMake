#pragma once

#include <span>
#include "10_Core/VuCommon.h"


namespace Vu {
    struct VuGraphicsPipeline {
        VkPipeline pipeline;

        void initGraphicsPipeline(
            const VkPipelineLayout pipelineLayout,
            const VkShaderModule vertShaderModule,
            const VkShaderModule fragShaderModule,
            const std::span<VkVertexInputBindingDescription> bindingDescriptions,
            const std::span<VkVertexInputAttributeDescription> attributeDescriptions,
            const VkRenderPass renderPass);

        void Dispose() const;

        static VkPipelineDepthStencilStateCreateInfo fillDepthStencilCreateInfo(bool        bDepthTest,
                                                                                bool        bDepthWrite,
                                                                                VkCompareOp compareOp);
    };
}
