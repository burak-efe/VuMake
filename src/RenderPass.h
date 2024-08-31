#pragma once

#include "EngineContext.h"
#include "SwapChain.h"
#include "VulkanUtils.h"
#include "vulkan/vulkan.h"

class RenderPass {
public:
    VkRenderPass VkRenderPass;
    VkFormat DepthFormat;
    VkImage DepthImage;
    VkImageView DepthImageView;
    VmaAllocation DepthImageAllocation;
    VkPipelineDepthStencilStateCreateInfo DepthStencil;

    void CreateRenderPass(const VuMake::SwapChain &swapchain) {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapchain.swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        VkAttachmentDescription depth_attachment = {};
        // Depth attachment
        depth_attachment.flags = 0;
        depth_attachment.format = DepthFormat;
        depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_attachment_ref = {};
        depth_attachment_ref.attachment = 1;
        depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depth_attachment_ref;


        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkSubpassDependency depth_dependency = {};
        depth_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        depth_dependency.dstSubpass = 0;
        depth_dependency.srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depth_dependency.srcAccessMask = 0;
        depth_dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        depth_dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        //array of 2 attachments, one for the color, and other for depth
        VkAttachmentDescription attachments[2] = {colorAttachment, depth_attachment};

        VkSubpassDependency dependencies[2] = {dependency, depth_dependency};


        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = &attachments[0];
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 2;
        renderPassInfo.pDependencies = &dependencies[0];

        VK_CHECK(vkCreateRenderPass(EngineContext::Device, &renderPassInfo, nullptr, &VkRenderPass));
    };


    VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;

        info.imageType = VK_IMAGE_TYPE_2D;

        info.format = format;
        info.extent = extent;

        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = usageFlags;

        return info;
    }

    VkImageViewCreateInfo ImageviewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
        //build a image-view for the depth image to use for rendering
        VkImageViewCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext = nullptr;

        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.image = image;
        info.format = format;
        info.subresourceRange.baseMipLevel = 0;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount = 1;
        info.subresourceRange.aspectMask = aspectFlags;

        return info;
    }

    void CreateDepthImage(const VuMake::SwapChain &swapchain) {
        //depth image size will match the window
        VkExtent3D depthImageExtent = {
            swapchain.swapChainExtent.width,
            swapchain.swapChainExtent.height,
            1
        };

        //hardcoding the depth format to 32 bit float
        DepthFormat = VK_FORMAT_D32_SFLOAT;

        //the depth image will be an image with the format we selected and Depth Attachment usage flag
        VkImageCreateInfo dimg_info = ImageCreateInfo(DepthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                                        depthImageExtent);

        //for the depth image, we want to allocate it from GPU local memory
        VmaAllocationCreateInfo dimg_allocinfo = {};
        dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        dimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        //allocate and create the image
        vmaCreateImage(EngineContext::VmaAllocator, &dimg_info, &dimg_allocinfo, &DepthImage, &DepthImageAllocation, nullptr);

        //build an image-view for the depth image to use for rendering
        VkImageViewCreateInfo dview_info = ImageviewCreateInfo(DepthFormat, DepthImage,
                                                                 VK_IMAGE_ASPECT_DEPTH_BIT);

        VK_CHECK(vkCreateImageView(EngineContext::Device, &dview_info, nullptr, &DepthImageView))
    }

    static VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo(bool bDepthTest, bool bDepthWrite,
                                                                 VkCompareOp compareOp) {
        VkPipelineDepthStencilStateCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        info.pNext = nullptr;

        info.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
        info.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
        info.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
        info.depthBoundsTestEnable = VK_FALSE;
        info.minDepthBounds = 0.0f; // Optional
        info.maxDepthBounds = 1.0f; // Optional
        info.stencilTestEnable = VK_FALSE;

        return info;
    }

    void Dispose() {
        vkDestroyRenderPass(EngineContext::Device, VkRenderPass, nullptr);
        vkDestroyImageView(EngineContext::Device, DepthImageView, nullptr);
        vmaDestroyImage(EngineContext::VmaAllocator, DepthImage, DepthImageAllocation);
    }
};
