#pragma once
#include "Common.h"
#include "VuDepthStencil.h"
#include "VuRenderPass.h"
#include "VuTypes.h"

namespace Vu {
    struct VuSwapChain {
    private:
        void CreateSwapChain(VkSurfaceKHR surfaceKHR);

        void CreateImageViews(VkDevice device);

        void CreateFramebuffers();

    public:
        VkSwapchainKHR swapChain;
        VuRenderPass renderPass;
        VuDepthStencil depthStencil;

        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;
        std::vector<VkFramebuffer> framebuffers;

        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        uint32_t imageCount;
        uint32_t queueNodeIndex = UINT32_MAX;

        void InitSwapChain(VkSurfaceKHR surface);

        void Dispose();

        void ResetSwapChain(VkSurfaceKHR surface);

        void BeginRenderPass(VkCommandBuffer commandBuffer, uint32 frameIndex);

        void EndRenderPass(VkCommandBuffer commandBuffer);

        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    };
}
