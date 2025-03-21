#pragma once

#include "10_Core/VuCommon.h"

namespace Vu
{
    struct VuGraphicsPipeline
    {
        VkDevice   device;
        VkPipeline pipeline;

        void initGraphicsPipeline(const VkDevice         device,
                                  const VkPipelineLayout pipelineLayout,
                                  const VkShaderModule   vertShaderModule,
                                  const VkShaderModule   fragShaderModule,
                                  const VkRenderPass     renderPass);

        void uninit() const;

        static VkPipelineDepthStencilStateCreateInfo fillDepthStencilCreateInfo(bool        bDepthTest,
                                                                                bool        bDepthWrite,
                                                                                VkCompareOp compareOp);
    };
}
