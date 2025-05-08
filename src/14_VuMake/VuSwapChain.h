#pragma once

#include <stdint.h>                      // for uint32_t, UINT32_MAX

#include <memory>                        // for shared_ptr
#include <vector>                        // for vector

#include "12_VuMakeCore/VuCommon.h"
#include "08_LangUtils/TypeDefs.h"       // for u32
#include "12_VuMakeCore/VuRenderPass.h"  // for VuRenderPass
#include "12_VuMakeCore/VuTypes.h"       // for QueueFamilyIndices

namespace Vu
{


struct VuImage;
struct VuDevice;

struct VuSwapChain
{
    VuDevice*        vuDevice;
    vk::SurfaceKHR   surface;
    vk::SwapchainKHR swapChain{};

    VuRenderPass gBufferPass;
    VuRenderPass lightningPass;

    std::shared_ptr<VuImage> colorHnd;
    std::shared_ptr<VuImage> normalHnd;
    std::shared_ptr<VuImage> armHnd;
    std::shared_ptr<VuImage> depthStencilHnd;


    std::vector<vk::Image>     swapChainImages;
    std::vector<vk::ImageView> swapChainImageViews;

    std::vector<vk::Framebuffer> lightningFrameBuffers;
    std::vector<vk::Framebuffer> gPassFrameBuffers;

    vk::Format        colorFormat;
    vk::ColorSpaceKHR colorSpace;
    vk::Format        swapChainImageFormat;
    vk::Extent2D      swapChainExtent{};
    uint32_t          imageCount{};
    uint32_t          queueNodeIndex = UINT32_MAX;

public:
    VuSwapChain();

    VuSwapChain(VuDevice* vuDevice, vk::SurfaceKHR surface);

    void uninit();

    void resetSwapChain(vk::SurfaceKHR surface);

    void beginGBufferPass(vk::CommandBuffer commandBuffer, u32 frameIndex);
    void beginLightningPass(vk::CommandBuffer commandBuffer, u32 frameIndex);

    void endRenderPass(vk::CommandBuffer commandBuffer);

    static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

    static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

    static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

    static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

private:
    void createSwapChain(vk::SurfaceKHR surfaceKHR);

    void createImageViews(vk::Device device);

    void createFramebuffers();
};


}