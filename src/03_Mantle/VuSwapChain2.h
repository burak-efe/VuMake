#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "VuDevice.h"
#include "VuPhysicalDevice.h"

namespace Vu {

struct VuSwapChain2 {
  std::shared_ptr<VuDevice>            vuDevice    = {};
  std::shared_ptr<vk::raii::SurfaceKHR> surface     = {};
  vk::raii::SwapchainKHR                swapchain   = {nullptr};
  std::vector<vk::Image>                images      = {};
  vk::Format                            imageFormat = {};
  vk::Extent2D                          extend2D    = {};
  std::vector<vk::raii::ImageView>      imageViews  = {};

  VuSwapChain2() = default;
  VuSwapChain2(const std::shared_ptr<VuDevice>& vuDevice, const std::shared_ptr<vk::raii::SurfaceKHR>& surface)
      : vuDevice {vuDevice}, surface {surface} {
    std::shared_ptr<VuPhysicalDevice>& physicalDevice = vuDevice->vuPhysicalDevice;


    const auto& [capabilities, formats, presentModes] = physicalDevice->swapChainSupport;
    vk::Extent2D         extend                       = chooseSwapExtent(capabilities);
    vk::SurfaceFormatKHR surfaceFormat                = chooseSwapSurfaceFormat(formats);
    vk::PresentModeKHR   presentMode                  = chooseSwapPresentMode(presentModes);
    u32                  imageCount                   = capabilities.minImageCount;

    vk::SwapchainCreateInfoKHR scCreateInfo {};
    scCreateInfo.surface          = *surface;
    scCreateInfo.minImageCount    = imageCount;
    scCreateInfo.imageFormat      = surfaceFormat.format;
    scCreateInfo.imageColorSpace  = surfaceFormat.colorSpace;
    scCreateInfo.imageExtent      = extend;
    scCreateInfo.imageArrayLayers = 1;
    scCreateInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment;

    u32 queueFamilyIndices[] = {physicalDevice->indices.graphicsFamily.value(),
                                physicalDevice->indices.presentFamily.value()};

    if (physicalDevice->indices.graphicsFamily != physicalDevice->indices.presentFamily) {
      scCreateInfo.imageSharingMode      = vk::SharingMode::eConcurrent;
      scCreateInfo.queueFamilyIndexCount = 2;
      scCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
      scCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    }

    scCreateInfo.preTransform   = capabilities.currentTransform;
    scCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    scCreateInfo.presentMode    = presentMode;
    scCreateInfo.clipped        = VK_TRUE;
    scCreateInfo.oldSwapchain   = VK_NULL_HANDLE;

    auto swapchainOrErr = vuDevice->device.createSwapchainKHR(scCreateInfo);
    // todo
    throw_if_unexpected(swapchainOrErr);
    swapchain = std::move(swapchainOrErr.value());
    images    = swapchain.getImages();

    imageFormat = surfaceFormat.format;
    extend2D    = extend;

    {
      imageViews.clear();

      for (size_t i = 0; i < images.size(); i++) {
        vk::ImageViewCreateInfo createInfo {};
        createInfo.image        = images[i];
        createInfo.viewType     = vk::ImageViewType::e2D;
        createInfo.format       = surfaceFormat.format;
        createInfo.components.r = vk::ComponentSwizzle::eIdentity;
        createInfo.components.g = vk::ComponentSwizzle::eIdentity;
        createInfo.components.b = vk::ComponentSwizzle::eIdentity;
        createInfo.components.a = vk::ComponentSwizzle::eIdentity;

        createInfo.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        auto imageViewOrErr = vuDevice->device.createImageView(createInfo);
        // todo
        throw_if_unexpected(imageViewOrErr);
        imageViews.emplace_back(std::move(imageViewOrErr.value()));
      }
    }
  }

  static vk::SurfaceFormatKHR
  chooseSwapSurfaceFormat(std::span<const vk::SurfaceFormatKHR> availableFormats) {
    for (const auto& format : availableFormats) {
      if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
        return format;
      }
    }
    return availableFormats[0];
  }

  static vk::PresentModeKHR
  chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
    // TODO present mode
    //  for (const auto& availablePresentMode: availablePresentModes) {
    //      if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
    //          return availablePresentMode;
    //      }
    //  }
    return vk::PresentModeKHR::eImmediate;
    return vk::PresentModeKHR::eMailbox;
    return vk::PresentModeKHR::eFifo;
  }

  [[nodiscard]] static vk::Extent2D
  chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) { return capabilities.currentExtent; }
    // int width, height;
    // SDL_GetWindowSize(ctx::window, &width, &height);
    // std::cout << "width: " << width << std::endl;

    // todo
    vk::Extent2D actualExtent = capabilities.currentExtent;

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
  }
};

} // namespace Vu