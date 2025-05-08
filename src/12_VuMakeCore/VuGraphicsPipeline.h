#pragma once

#include <span>                  // for span

#include <vulkan/vulkan_core.h>  // for vk::Device, vk::ShaderModule, vk::CompareOp

namespace Vu
{
    struct VuGraphicsPipeline
    {
        vk::Device   device;
        vk::Pipeline pipeline;

        void initGraphicsPipeline(vk::Device                                       device,
                                  vk::PipelineLayout                               pipelineLayout,
                                  vk::ShaderModule                                 vertShaderModule,
                                  vk::ShaderModule                                 fragShaderModule,
                                  vk::RenderPass                                   renderPass,
                                  std::span<vk::PipelineColorBlendAttachmentState> colorBlends
        );

        void uninit() const;

        static vk::PipelineDepthStencilStateCreateInfo fillDepthStencilCreateInfo(bool bDepthTest,
            bool                                                                     bDepthWrite,
            vk::CompareOp                                                              compareOp);
    };
}
