#pragma once
#include "Common.h"
#include "VuDepthStencil.h"
#include "VuRenderPass.h"
#include "VuUtils.h"


namespace Vu {
    class VuSwapChain {
    private:
        void createSwapChain(VkSurfaceKHR surfaceKHR);

        void createImageViews(VkDevice device);

        void createFramebuffers();

    public:
        VkSwapchainKHR swapChain;
        VuRenderPass renderPass;
        VuDepthStencil depthStencil;

        std::vector<VkImage> SwapChainImages;
        std::vector<VkImageView> SwapChainImageViews;
        std::vector<VkFramebuffer> Framebuffers;

        VkFormat ColorFormat;
        VkColorSpaceKHR ColorSpace;
        VkFormat SwapChainImageFormat;
        VkExtent2D SwapChainExtent;
        uint32_t ImageCount;
        uint32_t QueueNodeIndex = UINT32_MAX;

        void InitSwapChain(VkSurfaceKHR surface) {
            createSwapChain(surface);
            createImageViews(Vu::Device);
            depthStencil.Init(SwapChainExtent);
            renderPass.Init(SwapChainImageFormat, depthStencil.depthFormat);
            createFramebuffers();
        }

        void Dispose() {
            for (auto imageView: SwapChainImageViews) {
                vkDestroyImageView(Vu::Device, imageView, nullptr);
            }
            for (auto framebuffer: Framebuffers) {
                vkDestroyFramebuffer(Device, framebuffer, nullptr);
            }

            renderPass.Dispose();

            depthStencil.Dispose();
            vkDestroySwapchainKHR(Vu::Device, swapChain, nullptr);
        }


        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

        void ResetSwapChain(VkSurfaceKHR surface) {
            for (auto imageView: SwapChainImageViews) {
                vkDestroyImageView(Vu::Device, imageView, nullptr);
            }
            for (auto framebuffer: Framebuffers) {
                vkDestroyFramebuffer(Device, framebuffer, nullptr);
            }
            depthStencil.Dispose();
            vkDestroySwapchainKHR(Vu::Device, swapChain, nullptr);

            createSwapChain(surface);
            createImageViews(Device);
            depthStencil.Init(SwapChainExtent);
            createFramebuffers();
        }


        void BeginRenderPass(VkCommandBuffer commandBuffer, uint32 frameIndex) {

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {{0.02f, 0.02f, 0.02f, 1.0f}};
            clearValues[1].depthStencil = {1.0f, 0};

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass.renderPass;
            renderPassInfo.framebuffer = Framebuffers[frameIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = SwapChainExtent;
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }

        void EndRenderPass(VkCommandBuffer commandBuffer) {
            vkCmdEndRenderPass(commandBuffer);
        }
    };
}
