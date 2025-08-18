#pragma once
#include <vector>

#include "02_OuterCore/VuCommon.h"

namespace Vu {
struct VuInstance;

struct VuQueueFamilyIndices {
  uint32_t graphicsFamily {};
  uint32_t presentFamily {};

  VuQueueFamilyIndices(std::nullptr_t);

  static std::expected<VuQueueFamilyIndices, VkResult>
  make(const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface) noexcept;

private:
  VuQueueFamilyIndices();
};
// #####################################################################################################################

struct VuSwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR        capabilities {};
  std::vector<VkSurfaceFormatKHR> formats {};
  std::vector<VkPresentModeKHR>   presentModes {};

  static std::expected<VuSwapChainSupportDetails, VkResult>
  make(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface) noexcept;
};
// #####################################################################################################################

struct VuPhysicalDevice {
  std::shared_ptr<VuInstance>      m_vuInstance {nullptr};
  VkPhysicalDevice                 m_physicalDevice {nullptr};
  VuQueueFamilyIndices             m_indices {nullptr};
  VuSwapChainSupportDetails        m_swapChainSupport {};
  VkPhysicalDeviceProperties       m_properties {};
  VkPhysicalDeviceMemoryProperties m_memoryProperties {};
  VkPhysicalDeviceFeatures         m_features {};
  //--------------------------------------------------------------------------------------------------------------------
  VuPhysicalDevice()                        = default;
  VuPhysicalDevice(const VuPhysicalDevice&) = delete;
  VuPhysicalDevice&
  operator=(const VuPhysicalDevice&) = delete;

  VuPhysicalDevice(VuPhysicalDevice&& other) noexcept :
      m_vuInstance(std::move(other.m_vuInstance)),
      m_physicalDevice(other.m_physicalDevice),
      m_indices(std::move(other.m_indices)),
      m_swapChainSupport(std::move(other.m_swapChainSupport)),
      m_properties(other.m_properties),
      m_memoryProperties(other.m_memoryProperties),
      m_features(other.m_features) {
    other.m_physicalDevice = VK_NULL_HANDLE;
  }

  VuPhysicalDevice&
  operator=(VuPhysicalDevice&& other) noexcept {
    if (this != &other) {
      cleanup();
      m_vuInstance       = std::move(other.m_vuInstance);
      m_physicalDevice   = other.m_physicalDevice;
      m_indices          = std::move(other.m_indices);
      m_swapChainSupport = std::move(other.m_swapChainSupport);
      m_properties       = other.m_properties;
      m_memoryProperties = other.m_memoryProperties;
      m_features         = other.m_features;

      other.m_physicalDevice = VK_NULL_HANDLE;
    }
    return *this;
  }

  ~VuPhysicalDevice() { cleanup(); }

  SETUP_EXPECTED_WRAPPER(VuPhysicalDevice,
                         (std::shared_ptr<VuInstance> vuInstance,
                          const VkSurfaceKHR&         surface,
                          std::span<const char*>      enabledExtensions),
                         (vuInstance, surface, enabledExtensions))
private:
  void
  cleanup() {
    // No Vulkan destruction needed for VkPhysicalDevice; just reset shared_ptr
    m_physicalDevice = VK_NULL_HANDLE;
    m_vuInstance.reset();
  }
  //--------------------------------------------------------------------------------------------------------------------

  VuPhysicalDevice(std::shared_ptr<VuInstance> vuInstance,
                   const VkSurfaceKHR&         surface,
                   std::span<const char*>      enabledExtensions);

  static bool
  isSupported(const VkPhysicalDevice& phyDevice, const VkSurfaceKHR& surface, std::span<const char*> enabledExtensions);

  static bool
  isExtensionsSupported(const VkPhysicalDevice& device, std::span<const char*> requestedExtensions);
};
} // namespace Vu
