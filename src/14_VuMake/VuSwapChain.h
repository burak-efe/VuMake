#pragma once

#include "10_Core/VuCommon.h"
#include "12_VuMakeCore/VuTypes.h"
#include "12_VuMakeCore/VuImage.h"
#include "08_LangUtils/VuPoolManager.h"
#include "12_VuMakeCore/VuRenderPass.h"

namespace Vu
{
    struct VuSwapChain
    {
        VuDevice*      vuDevice;
        VkSurfaceKHR   surface;
        VkSwapchainKHR swapChain;

        VuRenderPass gBufferPass;
        VuRenderPass lightningPass;

        VuHnd<VuImage> colorHnd;
        VuHnd<VuImage> normalHnd;
        VuHnd<VuImage> armHnd;
        VuHnd<VuImage> depthStencilHnd;


        std::vector<VkImage>     swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        std::vector<VkFramebuffer> lightningFrameBuffers;
        std::vector<VkFramebuffer> gPassFrameBuffers;

        VkFormat        colorFormat;
        VkColorSpaceKHR colorSpace;
        VkFormat        swapChainImageFormat;
        VkExtent2D      swapChainExtent;
        uint32_t        imageCount;
        uint32_t        queueNodeIndex = UINT32_MAX;

    public:
        void init(VuDevice* vuDevice, VkSurfaceKHR surface);

        void uninit();

        void resetSwapChain(VkSurfaceKHR surface);

        void beginGBufferPass(VkCommandBuffer commandBuffer, u32 frameIndex);
        void beginLightningPass(VkCommandBuffer commandBuffer, u32 frameIndex);

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
