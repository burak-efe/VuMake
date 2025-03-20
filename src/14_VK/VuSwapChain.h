#pragma once
#include "10_Core/VuCommon.h"

#include "VuTypes.h"
#include "VuRenderPass.h"
#include "VuImage.h"

namespace Vu
{
    struct VuSwapChainCreateInfo
    {
        //VuDevice*        vuDevice;
        VkDevice         device;
        VkPhysicalDevice physicalDevice;
        VkSurfaceKHR     surface;
    };

    struct VuSwapChain
    {
    public:
        VuSwapChainCreateInfo lastCreateInfo;
        VkSwapchainKHR        swapChain;
        VuRenderPass          renderPass;
        VuImage               depthStencil;
        //VuDepthStencil depthStencil;

        std::vector<VkImage>       swapChainImages;
        std::vector<VkImageView>   swapChainImageViews;
        std::vector<VkFramebuffer> framebuffers;

        VkFormat        colorFormat;
        VkColorSpaceKHR colorSpace;
        VkFormat        swapChainImageFormat;
        VkExtent2D      swapChainExtent;
        uint32_t        imageCount;
        uint32_t        queueNodeIndex = UINT32_MAX;

        void init(const VuSwapChainCreateInfo& createInfo);

        void uninit();

        void resetSwapChain(VkSurfaceKHR surface);

        void beginRenderPass(VkCommandBuffer commandBuffer, uint32 frameIndex);

        void endRenderPass(VkCommandBuffer commandBuffer);

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    private:
        void createSwapChain(VkSurfaceKHR surfaceKHR);

        void createImageViews(VkDevice device);

        void createFramebuffers();
    };
}
