#pragma once

#include "02_OuterCore/VuCommon.h"

namespace Vu {
struct VuInstance;

struct VuQueueFamilyIndices {
  uint32_t graphicsFamily = {};
  uint32_t presentFamily  = {};

  VuQueueFamilyIndices(std::nullptr_t);

  static std::expected<VuQueueFamilyIndices, vk::Result>
  make(const vk::raii::PhysicalDevice& physDevice, const vk::raii::SurfaceKHR& surface) noexcept;

private:
  VuQueueFamilyIndices();
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VuSwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR        capabilities = {};
  std::vector<vk::SurfaceFormatKHR> formats      = {};
  std::vector<vk::PresentModeKHR>   presentModes = {};

  static std::expected<VuSwapChainSupportDetails, vk::Result>
  make(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface) noexcept;
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VuPhysicalDevice {
  std::shared_ptr<VuInstance>        vuInstance       = {};
  vk::raii::PhysicalDevice           physicalDevice   = {nullptr};
  VuQueueFamilyIndices               indices          = {nullptr};
  VuSwapChainSupportDetails          swapChainSupport = {};
  vk::PhysicalDeviceProperties       properties       = {};
  vk::PhysicalDeviceMemoryProperties memoryProperties = {};
  vk::PhysicalDeviceFeatures         features         = {};

  static std::expected<VuPhysicalDevice, vk::Result>
  make(const std::shared_ptr<VuInstance>& vuInstance,
       const vk::raii::SurfaceKHR&        surface,
       std::span<const char*>             enabledExtensions) noexcept;

private:
  VuPhysicalDevice(const std::shared_ptr<VuInstance>& vuInstance,
                   const vk::raii::SurfaceKHR&        surface,
                   std::span<const char*>             enabledExtensions);

  static bool
  isSupported(const vk::raii::PhysicalDevice& phyDevice,
              const vk::raii::SurfaceKHR&     surface,
              std::span<const char*>          enabledExtensions);

  static bool
  isExtensionsSupported(const vk::raii::PhysicalDevice& device, std::span<const char*> requestedExtensions);
};
} // namespace Vu
