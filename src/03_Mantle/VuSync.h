#pragma once

#include "../02_OuterCore/VuCommon.h"

namespace Vu {

inline VkImageMemoryBarrier
ImageMemoryBarrier() {
  VkImageMemoryBarrier imageMemoryBarrier {};
  imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  return imageMemoryBarrier;
}

inline void
InsertImageMemoryBarrier(VkCommandBuffer         cmdbuffer,
                         VkImage                 image,
                         VkAccessFlags           srcAccessMask,
                         VkAccessFlags           dstAccessMask,
                         VkImageLayout           oldImageLayout,
                         VkImageLayout           newImageLayout,
                         VkPipelineStageFlags    srcStageMask,
                         VkPipelineStageFlags    dstStageMask,
                         VkImageSubresourceRange subresourceRange) {
  VkImageMemoryBarrier imageMemoryBarrier = ImageMemoryBarrier();
  imageMemoryBarrier.srcAccessMask        = srcAccessMask;
  imageMemoryBarrier.dstAccessMask        = dstAccessMask;
  imageMemoryBarrier.oldLayout            = oldImageLayout;
  imageMemoryBarrier.newLayout            = newImageLayout;
  imageMemoryBarrier.image                = image;
  imageMemoryBarrier.subresourceRange     = subresourceRange;

  vkCmdPipelineBarrier(
      cmdbuffer, srcStageMask, dstStageMask, ZERO_FLAG, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}
} // namespace Vu
