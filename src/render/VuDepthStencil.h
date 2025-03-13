#pragma once

#include "Common.h"
// #include "VuUtils.h"
// #include "VuCtx.h"

namespace Vu
{
    //struct VuDepthStencil
    //{
        // VkImage       image;
        // VkImageView   imageView;
        // VmaAllocation allocation;
        // VkFormat      depthFormat;
        //
        // void init(VkExtent2D extent2D, VkFormat format = VK_FORMAT_D32_SFLOAT_S8_UINT) {
        //     ZoneScoped;
        //     VkExtent3D depthImageExtent = {
        //         extent2D.width,
        //         extent2D.height,
        //         1
        //     };
        //
        //     depthFormat                       = format;
        //     VkImageCreateInfo imageCreateInfo = fillImageCreateInfo(depthFormat,
        //                                                             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        //                                                             depthImageExtent);
        //
        //     //for the depth image, we want to allocate it from GPU local memory
        //     VmaAllocationCreateInfo dimg_allocinfo = {};
        //     dimg_allocinfo.usage                   = VMA_MEMORY_USAGE_GPU_ONLY;
        //     dimg_allocinfo.requiredFlags           = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        //
        //     //allocate and create the image
        //     vmaCreateImage(ctx::vuDevice->vma, &imageCreateInfo, &dimg_allocinfo, &image, &allocation, nullptr);
        //
        //     //build an image-view for the depth image to use for rendering
        //     VkImageViewCreateInfo imageViewCreateInfo = fillImageViewCreateInfo(depthFormat, image, VK_IMAGE_ASPECT_DEPTH_BIT);
        //
        //     VkCheck(vkCreateImageView(ctx::vuDevice->device, &imageViewCreateInfo, nullptr, &imageView));
        //
        //     giveDebugName(ctx::vuDevice->device, VK_OBJECT_TYPE_IMAGE, image, "Depth Image");
        //     giveDebugName(ctx::vuDevice->device, VK_OBJECT_TYPE_IMAGE_VIEW, imageView, "Depth ImageView");
        // }
        //
        // void uninit() {
        //     vkDestroyImageView(ctx::vuDevice->device, imageView, nullptr);
        //     vmaDestroyImage(ctx::vuDevice->vma, image, allocation);
        // }


    //};
}
