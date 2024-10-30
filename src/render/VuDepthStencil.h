#pragma once

#include "Common.h"
#include "VuCtx.h"
#include "VuUtils.h"

namespace Vu {
    struct VuDepthStencil {
        VkImage image;
        VkImageView imageView;
        VmaAllocation allocation;
        VkFormat depthFormat;

        void Init(VkExtent2D extent2D) {
            //depth image size will match the window
            VkExtent3D depthImageExtent = {
                extent2D.width,
                extent2D.height,
                1
            };

            //hardcoding the depth format to 32 bit float
            depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

            //the depth image will be an image with the format we selected and Depth Attachment usage flag
            VkImageCreateInfo dimg_info = Vu::CreateImageCreateInfo(depthFormat,
                                                                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                    depthImageExtent);

            //for the depth image, we want to allocate it from GPU local memory
            VmaAllocationCreateInfo dimg_allocinfo = {};
            dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
            dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

            //allocate and create the image
            vmaCreateImage(ctx::vma, &dimg_info, &dimg_allocinfo, &image, &allocation, nullptr);

            //build an image-view for the depth image to use for rendering
            VkImageViewCreateInfo dview_info = Vu::CreateImageViewCreateInfo(depthFormat, image,
                                                                             VK_IMAGE_ASPECT_DEPTH_BIT);

            VkCheck(vkCreateImageView(ctx::device, &dview_info, nullptr, &imageView));
        }

        void Dispose() {
            vkDestroyImageView(ctx::device, imageView, nullptr);
            vmaDestroyImage(ctx::vma, image, allocation);
        }

        static VkPipelineDepthStencilStateCreateInfo CreateDepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite,
                                                                                  VkCompareOp compareOp) {
            VkPipelineDepthStencilStateCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
            info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
            info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
            info.depthBoundsTestEnable = VK_FALSE;
            info.minDepthBounds = 0.0f; // Optional
            info.maxDepthBounds = 1.0f; // Optional
            //info.stencilTestEnable = VK_TRUE;

            return info;
        }
    };
}
