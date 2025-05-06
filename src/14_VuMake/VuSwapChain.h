#pragma once

#include <stdint.h>                      // for uint32_t, UINT32_MAX
#include <vulkan/vulkan_core.h>          // for VkSurfaceKHR, VkCommandBuffer
#include <memory>                        // for shared_ptr
#include <vector>                        // for vector

#include "08_LangUtils/TypeDefs.h"       // for u32
#include "12_VuMakeCore/VuRenderPass.h"  // for VuRenderPass
#include "12_VuMakeCore/VuTypes.h"       // for QueueFamilyIndices

namespace Vu
{
    struct VuImage;
    struct VuDevice;

    struct VuSwapChain
    {
        VuDevice*      vuDevice;
        VkSurfaceKHR   surface;
        VkSwapchainKHR swapChain{};

        VuRenderPass gBufferPass;
        VuRenderPass lightningPass;

        std::shared_ptr<VuImage> colorHnd;
        std::shared_ptr<VuImage> normalHnd;
        std::shared_ptr<VuImage> armHnd;
        std::shared_ptr<VuImage> depthStencilHnd;


        std::vector<VkImage>     swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        std::vector<VkFramebuffer> lightningFrameBuffers;
        std::vector<VkFramebuffer> gPassFrameBuffers;

        VkFormat        colorFormat;
        VkColorSpaceKHR colorSpace;
        VkFormat        swapChainImageFormat;
        VkExtent2D      swapChainExtent{};
        uint32_t        imageCount{};
        uint32_t        queueNodeIndex = UINT32_MAX;

    public:
        VuSwapChain();

        VuSwapChain(VuDevice* vuDevice, VkSurfaceKHR surface);

        //void init(VuDevice* vuDevice, VkSurfaceKHR surface);

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
