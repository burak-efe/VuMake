#pragma once
#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>
#include "Vu.h"
#include "VuSwapChain.h"
#include "VuUtils.h"

class VuDepthStencil {
public:
    VkImage Image;
    VkImageView View;
    VmaAllocation VmaAllocation;
    VkFormat DepthFormat;
    VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo;

    void Create(const Vu::VuSwapChain &swapchain) {
        //depth image size will match the window
        VkExtent3D depthImageExtent = {
            swapchain.swapChainExtent.width,
            swapchain.swapChainExtent.height,
            1
        };

        //hardcoding the depth format to 32 bit float
        DepthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;

        //the depth image will be an image with the format we selected and Depth Attachment usage flag
        VkImageCreateInfo dimg_info = Vu::CreateImageCreateInfo(DepthFormat,
                                                                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                                depthImageExtent);

        //for the depth image, we want to allocate it from GPU local memory
        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        //allocate and create the image
        vmaCreateImage(Vu::VmaAllocator, &dimg_info, &dimg_allocinfo, &Image, &VmaAllocation, nullptr);

        //build an image-view for the depth image to use for rendering
        VkImageViewCreateInfo dview_info = Vu::CreateImageViewCreateInfo(DepthFormat, Image,
                                                                         VK_IMAGE_ASPECT_DEPTH_BIT);

        VK_CHECK(vkCreateImageView(Vu::Device, &dview_info, nullptr, &View));
    }

    void Dispose() {
        vkDestroyImageView(Vu::Device, View, nullptr);
        vmaDestroyImage(Vu::VmaAllocator, Image, VmaAllocation);
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