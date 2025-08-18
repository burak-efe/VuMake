#pragma once

#include "02_OuterCore/VuCommon.h"
#include "VuDevice.h"

namespace Vu {
struct VuDevice;
struct VuSurface;

struct VuSwapChain {
  std::shared_ptr<VuDevice>  m_vuDevice {nullptr};
  std::shared_ptr<VuSurface> m_vuSurface {nullptr};
  VkSwapchainKHR             m_swapchain {nullptr};
  std::vector<VkImage>       m_images {};
  std::vector<VkImageView>   m_imageViews {};
  VkFormat                   m_imageFormat {};
  VkExtent2D                 m_extend2D {};

  static VkSurfaceFormatKHR
  chooseSwapSurfaceFormat(std::span<const VkSurfaceFormatKHR> availableFormats);

  static VkPresentModeKHR
  chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

  [[nodiscard]] static VkExtent2D
  chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

  //--------------------------------------------------------------------------------------------------------------------
  VuSwapChain()                   = default;
  VuSwapChain(const VuSwapChain&) = delete;
  VuSwapChain&
  operator=(const VuSwapChain&) = delete;

  VuSwapChain(VuSwapChain&& other) noexcept :
      m_vuDevice(std::move(other.m_vuDevice)),
      m_vuSurface(std::move(other.m_vuSurface)),
      m_swapchain(other.m_swapchain),
      m_images(std::move(other.m_images)),
      m_imageViews(std::move(other.m_imageViews)),
      m_imageFormat(other.m_imageFormat),
      m_extend2D(other.m_extend2D) {
    other.m_swapchain = VK_NULL_HANDLE;
  }

  VuSwapChain&
  operator=(VuSwapChain&& other) noexcept {
    if (this != &other) {
      cleanup();
      m_vuDevice    = std::move(other.m_vuDevice);
      m_vuSurface     = std::move(other.m_vuSurface);
      m_swapchain   = other.m_swapchain;
      m_images      = std::move(other.m_images);
      m_imageViews  = std::move(other.m_imageViews);
      m_imageFormat = other.m_imageFormat;
      m_extend2D    = other.m_extend2D;

      other.m_swapchain = VK_NULL_HANDLE;
    }
    return *this;
  }

  ~VuSwapChain() { cleanup(); }

  SETUP_EXPECTED_WRAPPER(VuSwapChain,
                         (std::shared_ptr<VuDevice> vuDevice, std::shared_ptr<VuSurface> surface),
                         (vuDevice, surface))
private:
  void
  cleanup() {
    for (auto view : m_imageViews) {
      if (view != VK_NULL_HANDLE) { vkDestroyImageView(m_vuDevice->m_device, view, nullptr); }
    }
    m_imageViews.clear();

    if (m_swapchain != VK_NULL_HANDLE) {
      vkDestroySwapchainKHR(m_vuDevice->m_device, m_swapchain, nullptr);
      m_swapchain = VK_NULL_HANDLE;
    }
    m_images.clear();

    m_vuDevice.reset();
    m_vuSurface.reset();
  }
  //--------------------------------------------------------------------------------------------------------------------

private:
  VuSwapChain(std::shared_ptr<VuDevice> vuDevice, std::shared_ptr<VuSurface> surface);
};

} // namespace Vu