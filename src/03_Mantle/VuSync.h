#pragma once

#include "vulkan/vulkan.hpp"

namespace Vu
{

inline vk::ImageMemoryBarrier ImageMemoryBarrier()
{
    vk::ImageMemoryBarrier imageMemoryBarrier{};
    imageMemoryBarrier.sType               = vk::StructureType::eImageMemoryBarrier;
    imageMemoryBarrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    imageMemoryBarrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    return imageMemoryBarrier;
}

inline void InsertImageMemoryBarrier(
        vk::CommandBuffer         cmdbuffer,
        vk::Image                 image,
        vk::AccessFlags           srcAccessMask,
        vk::AccessFlags           dstAccessMask,
        vk::ImageLayout           oldImageLayout,
        vk::ImageLayout           newImageLayout,
        vk::PipelineStageFlags    srcStageMask,
        vk::PipelineStageFlags    dstStageMask,
        vk::ImageSubresourceRange subresourceRange)
{

    vk::ImageMemoryBarrier imageMemoryBarrier = ImageMemoryBarrier();
    imageMemoryBarrier.srcAccessMask          = srcAccessMask;
    imageMemoryBarrier.dstAccessMask          = dstAccessMask;
    imageMemoryBarrier.oldLayout              = oldImageLayout;
    imageMemoryBarrier.newLayout              = newImageLayout;
    imageMemoryBarrier.image                  = image;
    imageMemoryBarrier.subresourceRange       = subresourceRange;

    cmdbuffer.pipelineBarrier(
            srcStageMask, dstStageMask,
            {},
            nullptr, nullptr,
            imageMemoryBarrier
            );
}


}
