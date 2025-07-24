#pragma once

namespace Vu {
struct VuDevice;

struct VuSwapChain {
  std::shared_ptr<VuDevice>             vuDevice    = {};
  std::shared_ptr<vk::raii::SurfaceKHR> surface     = {};
  vk::raii::SwapchainKHR                swapchain   = {nullptr};
  std::vector<vk::Image>                images      = {};
  vk::Format                            imageFormat = {};
  vk::Extent2D                          extend2D    = {};
  std::vector<vk::raii::ImageView>      imageViews  = {};

  VuSwapChain();
  VuSwapChain(const std::shared_ptr<VuDevice>& vuDevice, const std::shared_ptr<vk::raii::SurfaceKHR>& surface);

  static vk::SurfaceFormatKHR
  chooseSwapSurfaceFormat(std::span<const vk::SurfaceFormatKHR> availableFormats);

  static vk::PresentModeKHR
  chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

  [[nodiscard]] static vk::Extent2D
  chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);
};

} // namespace Vu