#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "VuDevice2.h"
#include "VuPhysicalDevice.h"

namespace Vu {

struct VuSwapChain2 {
  vk::raii::SwapchainKHR                swapchain   = {nullptr};
  std::vector<vk::Image>                images      = {};
  std::shared_ptr<vk::raii::SurfaceKHR> surface     = {};
  vk::Format                            imageFormat = {};
  vk::Extent2D                          extend2D    = {};

  VuSwapChain2(const VuPhysicalDevice&                      vuPhysicalDevice,
               const VuDevice2&                             vuDevice,
               const std::shared_ptr<vk::raii::SurfaceKHR>& surface)
      : surface {surface} {

    const auto& [capabilities, formats, presentModes] = vuPhysicalDevice.swapChainSupport;
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

    u32 queueFamilyIndices[] = {vuPhysicalDevice.indices.graphicsFamily.value(),
                                vuPhysicalDevice.indices.presentFamily.value()};

    if (vuPhysicalDevice.indices.graphicsFamily != vuPhysicalDevice.indices.presentFamily) {
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

    auto swapchainOrErr = vuDevice.device.createSwapchainKHR(scCreateInfo);
    // todo
    throw_if_unexpected(swapchainOrErr);
    swapchain = std::move(swapchainOrErr.value());
    images    = swapchain.getImages();

    imageFormat = surfaceFormat.format;
    extend2D    = extend;
  }

  // void
  // resetSwapChain(vk::SurfaceKHR surface) {
  //   for (auto imageView : swapChainImageViews) {
  //     vkDestroyImageView(vuDevice->device, imageView, nullptr);
  //   }
  //   for (auto framebuffer : lightningFrameBuffers) {
  //     vkDestroyFramebuffer(vuDevice->device, framebuffer, nullptr);
  //   }
  //
  //   vkDestroySwapchainKHR(vuDevice->device, swapChain, nullptr);
  //   createSwapChain(surface);
  //   createImageViews(vuDevice->device);
  //   createFramebuffers();
  // }

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

  vk::Extent2D
  chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const {
    if (capabilities.currentExtent.width != std::numeric_limits<u32>::max()) { return capabilities.currentExtent; }
    // int width, height;
    // SDL_GetWindowSize(ctx::window, &width, &height);
    // std::cout << "width: " << width << std::endl;

    // todo
    vk::Extent2D actualExtent = extend2D;

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
  }
};

} // namespace Vu