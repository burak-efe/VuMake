#pragma once
#include <vector>
#include "Common.h"
#include "VuDepthStencil.h"
#include "VuUtils.h"


namespace Vu {
    class VuSwapChain {
    private:
        void createSwapChain(VkSurfaceKHR surfaceKHR);

        void createImageViews(VkDevice device);

        void createRenderPass(VkFormat depthFormat);

        void createFramebuffers();

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

        void InitSwapChain(VkSurfaceKHR surface) {
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

        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

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
