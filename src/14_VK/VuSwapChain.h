#pragma once
#include "10_Core/VuCommon.h"

#include "VuTypes.h"
#include "VuRenderPass.h"
#include "VuImage.h"
#include "VuPools.h"

namespace Vu
{
    struct VuSwapChain
    {
    public:
        VuDevice*          vuDevice;
        VkSurfaceKHR       surface;
        VkSwapchainKHR     swapChain;
        VuRenderPass       renderPass;
        VuHandle2<VuImage> depthStencilH;


        std::vector<VkImage>       swapChainImages;
        std::vector<VkImageView>   swapChainImageViews;
        std::vector<VkFramebuffer> framebuffers;

        VkFormat        colorFormat;
        VkColorSpaceKHR colorSpace;
        VkFormat        swapChainImageFormat;
        VkExtent2D      swapChainExtent;
        uint32_t        imageCount;
        uint32_t        queueNodeIndex = UINT32_MAX;

        void init(VuDevice* vuDevice, VkSurfaceKHR surface);

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
