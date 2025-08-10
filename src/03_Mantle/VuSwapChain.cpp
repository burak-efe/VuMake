#include "VuSwapChain.h"

#include "VuDevice.h"
#include "VuPhysicalDevice.h"

namespace Vu {
VuSwapChain::VuSwapChain(std::shared_ptr<VuDevice> vuDevice, std::shared_ptr<VkSurfaceKHR> surface) :
    m_vuDevice {std::move(vuDevice)},
    m_surface {std::move(surface)} {

  auto& physicalDevice = m_vuDevice->m_vuPhysicalDevice;

  const auto& [capabilities, formats, presentModes] = physicalDevice->m_swapChainSupport;
  VkExtent2D         extend                         = chooseSwapExtent(capabilities);
  VkSurfaceFormatKHR surfaceFormat                  = chooseSwapSurfaceFormat(formats);
  VkPresentModeKHR   presentMode                    = chooseSwapPresentMode(presentModes);
  uint32_t           imageCount                     = capabilities.minImageCount;

  VkSwapchainCreateInfoKHR swapChainCreateInfo {};
  swapChainCreateInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapChainCreateInfo.surface          = *this->m_surface;
  swapChainCreateInfo.minImageCount    = imageCount;
  swapChainCreateInfo.imageFormat      = surfaceFormat.format;
  swapChainCreateInfo.imageColorSpace  = surfaceFormat.colorSpace;
  swapChainCreateInfo.imageExtent      = extend;
  swapChainCreateInfo.imageArrayLayers = 1;
  swapChainCreateInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  uint32_t queueFamilyIndices[] = {physicalDevice->m_indices.graphicsFamily, physicalDevice->m_indices.presentFamily};

  if (physicalDevice->m_indices.graphicsFamily != physicalDevice->m_indices.presentFamily) {
    swapChainCreateInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
    swapChainCreateInfo.queueFamilyIndexCount = 2;
    swapChainCreateInfo.pQueueFamilyIndices   = queueFamilyIndices;
  } else {
    swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  }

  swapChainCreateInfo.preTransform   = capabilities.currentTransform;
  swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  swapChainCreateInfo.presentMode    = presentMode;
  swapChainCreateInfo.clipped        = VK_TRUE;
  swapChainCreateInfo.oldSwapchain   = VK_NULL_HANDLE;

  VkResult swapChainRes =
      vkCreateSwapchainKHR(m_vuDevice->m_device, &swapChainCreateInfo, NO_ALLOC_CALLBACK, &this->m_swapchain);
  THROW_if_fail(swapChainRes);

  uint32_t swapChainImageCount = 0;
  VkResult swapChainImageQueryRes =
      vkGetSwapchainImagesKHR(m_vuDevice->m_device, m_swapchain, &swapChainImageCount, nullptr);
  THROW_if_fail(swapChainImageQueryRes);

  m_images.resize(swapChainImageCount);
  VkResult swapChainImageRes =
      vkGetSwapchainImagesKHR(m_vuDevice->m_device, m_swapchain, &swapChainImageCount, m_images.data());
  THROW_if_fail(swapChainImageRes);

  m_imageFormat = surfaceFormat.format;
  m_extend2D    = extend;

  {
    m_imageViews.resize(m_images.size());

    for (size_t i = 0; i < m_images.size(); i++) {
      VkImageViewCreateInfo createInfo {};
      createInfo.sType        = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      createInfo.image        = m_images[i];
      createInfo.viewType     = VK_IMAGE_VIEW_TYPE_2D;
      createInfo.format       = surfaceFormat.format;
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

      createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel   = 0;
      createInfo.subresourceRange.levelCount     = 1;
      createInfo.subresourceRange.baseArrayLayer = 0;
      createInfo.subresourceRange.layerCount     = 1;

      VkResult imageViewRes = vkCreateImageView(m_vuDevice->m_device, &createInfo, NO_ALLOC_CALLBACK, &m_imageViews[i]);

      THROW_if_fail(imageViewRes);
    }
  }
}

VkSurfaceFormatKHR
VuSwapChain::chooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> availableFormats) {
  for (const auto& format : availableFormats) {
    if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
      return format;
    }
  }
  return availableFormats[0];
}
VkPresentModeKHR
VuSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
  // TODO present mode
  //  for (const auto& availablePresentMode: availablePresentModes) {
  //      if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
  //          return availablePresentMode;
  //      }
  //  }
  return VK_PRESENT_MODE_IMMEDIATE_KHR;
  return VK_PRESENT_MODE_MAILBOX_KHR;
  return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D
VuSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) { return capabilities.currentExtent; }
  // int width, height;
  // SDL_GetWindowSize(ctx::window, &width, &height);
  // std::cout << "width: " << width << std::endl;

  // todo
  VkExtent2D actualExtent = capabilities.currentExtent;

  actualExtent.width =
      std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
  actualExtent.height =
      std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
  return actualExtent;
}
} // namespace Vu