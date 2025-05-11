#pragma once

#include <span>                  // for span

#include "vulkan/vulkan.hpp"


namespace Vu
{
struct VuGraphicsPipeline
{
    vk::raii::Pipeline pipeline = {nullptr};

    void
    initGraphicsPipeline(const vk::raii::Device&                          device,
                         const vk::raii::PipelineLayout&                  pipelineLayout,
                         const vk::raii::ShaderModule&                    vertShaderModule,
                         const vk::raii::ShaderModule&                    fragShaderModule,
                         const vk::raii::RenderPass&                      renderPass,
                         std::span<vk::PipelineColorBlendAttachmentState> colorBlends
    );


    static vk::PipelineDepthStencilStateCreateInfo
    fillDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite, vk::CompareOp compareOp);
};
}
