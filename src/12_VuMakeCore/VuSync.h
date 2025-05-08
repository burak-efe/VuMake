#pragma once

#include "10_Core/Common.h"


namespace VuSync {

    inline vk::ImageMemoryBarrier ImageMemoryBarrier() {
        vk::ImageMemoryBarrier imageMemoryBarrier{};
        imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        return imageMemoryBarrier;
    }

    inline void InsertImageMemoryBarrier(
        vk::CommandBuffer cmdbuffer,
        vk::Image image,
        vk::AccessFlags srcAccessMask,
        vk::AccessFlags dstAccessMask,
        vk::ImageLayout oldImageLayout,
        vk::ImageLayout newImageLayout,
        vk::PipelineStageFlags srcStageMask,
        vk::PipelineStageFlags dstStageMask,
        vk::ImageSubresourceRange subresourceRange) {

        vk::ImageMemoryBarrier imageMemoryBarrier = ImageMemoryBarrier();
        imageMemoryBarrier.srcAccessMask = srcAccessMask;
        imageMemoryBarrier.dstAccessMask = dstAccessMask;
        imageMemoryBarrier.oldLayout = oldImageLayout;
        imageMemoryBarrier.newLayout = newImageLayout;
        imageMemoryBarrier.image = image;
        imageMemoryBarrier.subresourceRange = subresourceRange;

        vkCmdPipelineBarrier(
            cmdbuffer,
            srcStageMask,
            dstStageMask,
            0,
            0,
            nullptr,
            0,
            nullptr,
            1,
            &imageMemoryBarrier
        );
    }

    //
    // inline void InsertImageMemoryBarrier2(
    //     vk::CommandBuffer cmdbuffer,
    //     vk::Image image,
    //     vk::AccessFlags2 srcAccessMask,
    //     vk::AccessFlags2 dstAccessMask,
    //     vk::ImageLayout oldImageLayout,
    //     vk::ImageLayout newImageLayout,
    //     vk::PipelineStageFlags2 srcStageMask,
    //     vk::PipelineStageFlags2 dstStageMask,
    //     vk::ImageSubresourceRange subresourceRange
    //
    // ) {
    //
    //     vk::ImageMemoryBarrier2 imageMemoryBarrier2{
    //         .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
    //         .srcStageMask = srcStageMask,
    //         .srcAccessMask = srcAccessMask,
    //         .dstStageMask = dstStageMask,
    //         .dstAccessMask = dstAccessMask,
    //         .oldLayout = oldImageLayout,
    //         .newLayout = newImageLayout,
    //         .srcQueueFamilyIndex = 0,
    //         .dstQueueFamilyIndex = 0,
    //         .image = image,
    //         .subresourceRange = subresourceRange
    //     };
    //
    //     vk::DependencyFlags dependencyFlags = 0;
    //
    //     vk::DependencyInfo dependInfo{
    //         .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
    //         .pNext = nullptr,
    //         .dependencyFlags = dependencyFlags,
    //         .memoryBarrierCount = 0,
    //         .pMemoryBarriers = nullptr,
    //         .bufferMemoryBarrierCount = 0,
    //         .pBufferMemoryBarriers = nullptr,
    //         .imageMemoryBarrierCount = 1,
    //         .pImageMemoryBarriers = &imageMemoryBarrier2,
    //     };
    //     vkCmdPipelineBarrier2(cmdbuffer, &dependInfo);
    // }
}
