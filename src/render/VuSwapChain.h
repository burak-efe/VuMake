#pragma once
#include <vector>
#include "Common.h"
#include "VuDepthStencil.h"
#include "VuUtils.h"
#include "GLFW/glfw3.h"

namespace Vu {

    class VuSwapChain {
    private:
        void createSwapChain(VkSurfaceKHR surfaceKHR) {
            surface = surfaceKHR;
            SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(Vu::PhysicalDevice, surfaceKHR);

            VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
            VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
            extend2d = ChooseSwapExtent(swapChainSupport.capabilities, window);

            uint32 imageCount = swapChainSupport.capabilities.minImageCount + 1;

            if (swapChainSupport.capabilities.maxImageCount > 0
                && imageCount > swapChainSupport.capabilities.maxImageCount) {
                imageCount = swapChainSupport.capabilities.maxImageCount;
            }

            VkSwapchainCreateInfoKHR createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
            createInfo.surface = surfaceKHR;
            createInfo.minImageCount = imageCount;
            createInfo.imageFormat = surfaceFormat.format;
            createInfo.imageColorSpace = surfaceFormat.colorSpace;
            createInfo.imageExtent = extend2d;
            createInfo.imageArrayLayers = 1;
            createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

            QueueFamilyIndices indices = FindQueueFamilies(Vu::PhysicalDevice, surfaceKHR);
            uint32 queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

            if (indices.graphicsFamily != indices.presentFamily) {
                createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
                createInfo.queueFamilyIndexCount = 2;
                createInfo.pQueueFamilyIndices = queueFamilyIndices;
            } else {
                createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            }

            createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
            createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
            createInfo.presentMode = presentMode;
            createInfo.clipped = VK_TRUE;
            createInfo.oldSwapchain = VK_NULL_HANDLE;

            VK_CHECK(vkCreateSwapchainKHR(Vu::Device, &createInfo, nullptr, &swapChain));

            vkGetSwapchainImagesKHR(Vu::Device, swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(Vu::Device, swapChain, &imageCount, swapChainImages.data());

            swapChainImageFormat = surfaceFormat.format;
            swapChainExtent = extend2d;
        }

        void createImageViews(VkDevice device);

        void createRenderPass(VkFormat depthFormat) {
            VkAttachmentDescription colorAttachment{};
            colorAttachment.format = swapChainImageFormat;
            colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

            VkAttachmentDescription depthAttachment{};
            depthAttachment.format = depthFormat;
            depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference colorAttachmentRef{};
            colorAttachmentRef.attachment = 0;
            colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depthAttachmentRef{};
            depthAttachmentRef.attachment = 1;
            depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkSubpassDescription subpass{};
            subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpass.colorAttachmentCount = 1;
            subpass.pColorAttachments = &colorAttachmentRef;
            subpass.pDepthStencilAttachment = &depthAttachmentRef;

            VkSubpassDependency dependency{};
            dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency.dstSubpass = 0;
            dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


            std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            renderPassInfo.pAttachments = attachments.data();
            renderPassInfo.subpassCount = 1;
            renderPassInfo.pSubpasses = &subpass;
            renderPassInfo.dependencyCount = 1;
            renderPassInfo.pDependencies = &dependency;

            VK_CHECK(vkCreateRenderPass(Vu::Device, &renderPassInfo, nullptr, &renderPass));
        }

        void createFramebuffers() {
            framebuffers.resize(swapChainImageViews.size());

            for (size_t i = 0; i < swapChainImageViews.size(); i++) {
                std::array attachments = {
                    swapChainImageViews[i],
                    depthStencil.View

                };

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = attachments.size();
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = swapChainExtent.width;
                framebufferInfo.height = swapChainExtent.height;
                framebufferInfo.layers = 1;

                VK_CHECK(vkCreateFramebuffer(Vu::Device, &framebufferInfo, nullptr, &framebuffers[i]));
            }
        }

    public:
        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        uint32_t imageCount;
        uint32_t queueNodeIndex = UINT32_MAX;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        VuDepthStencil depthStencil;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> framebuffers;
        VkExtent2D extend2d;
        VkSurfaceKHR surface;

        void InitSwapChain(GLFWwindow* window, VkSurfaceKHR surface) {
            createSwapChain(surface);
            createImageViews(Vu::Device);
            depthStencil.Init(extend2d);
            createRenderPass(depthStencil.DepthFormat);
            createFramebuffers();
        }

        void CleanupSwapchain() {
            for (auto imageView: swapChainImageViews) {
                vkDestroyImageView(Vu::Device, imageView, nullptr);
            }
            for (auto framebuffer : framebuffers) {
                vkDestroyFramebuffer(Device, framebuffer, nullptr);
            }

            vkDestroyRenderPass(Device, renderPass, nullptr);

            depthStencil.Dispose();
            vkDestroySwapchainKHR(Vu::Device, swapChain, nullptr);
        }

        void BeginRenderPass(VkCommandBuffer commandBuffer, uint32 frameIndex) {

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = framebuffers[frameIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        void EndRenderPass(VkCommandBuffer commandBuffer) {
            vkCmdEndRenderPass(commandBuffer);
        }

        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

        void Reset() {
            for (auto imageView: swapChainImageViews) {
                vkDestroyImageView(Vu::Device, imageView, nullptr);
            }
            for (auto framebuffer : framebuffers) {
                vkDestroyFramebuffer(Device, framebuffer, nullptr);
            }
            depthStencil.Dispose();
            vkDestroySwapchainKHR(Vu::Device, swapChain, nullptr);

            createSwapChain(surface);
            createImageViews(Device);
            depthStencil.Init(extend2d);
            createFramebuffers();
        }
    };
}
