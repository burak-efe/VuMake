#pragma once

#include <span>                  // for span

#include <vulkan/vulkan_core.h>  // for VkDevice, VkShaderModule, VkCompareOp

namespace Vu
{
    struct VuGraphicsPipeline
    {
        VkDevice   device;
        VkPipeline pipeline;

        void initGraphicsPipeline(VkDevice                                       device,
                                  VkPipelineLayout                               pipelineLayout,
                                  VkShaderModule                                 vertShaderModule,
                                  VkShaderModule                                 fragShaderModule,
                                  VkRenderPass                                   renderPass,
                                  std::span<VkPipelineColorBlendAttachmentState> colorBlends
        );

        void uninit() const;

        static VkPipelineDepthStencilStateCreateInfo fillDepthStencilCreateInfo(bool bDepthTest,
            bool                                                                     bDepthWrite,
            VkCompareOp                                                              compareOp);
    };
}
