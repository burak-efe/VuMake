
#include "VuPhysicalDevice.h"

#include <set>

#include "../02_OuterCore/VuCommon.h"
#include "VuInstance.h"

namespace Vu {
VuQueueFamilyIndices::VuQueueFamilyIndices(std::nullptr_t) {}

std::expected<VuQueueFamilyIndices, VkResult>
VuQueueFamilyIndices::make(const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface) noexcept {

  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, nullptr);
  std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(physDevice, &queueFamilyCount, queueFamilies.data());

  VuQueueFamilyIndices indices;
  int                  queueFamilyIndex = 0;

  std::optional<uint32_t> graphicsOrNull = {std::nullopt};
  std::optional<uint32_t> presentOrNull  = {std::nullopt};

  for (const auto& queueFamily : queueFamilies) {

    if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { graphicsOrNull = queueFamilyIndex; }

    VkBool32 surfaceSupported = false;
    VkResult supportQueryRes =
        vkGetPhysicalDeviceSurfaceSupportKHR(physDevice, queueFamilyIndex, surface, &surfaceSupported);

    if (surfaceSupported) { presentOrNull = queueFamilyIndex; }

    if (graphicsOrNull.has_value() && presentOrNull.has_value()) { break; }

    queueFamilyIndex++;
  }

  if (!graphicsOrNull.has_value() || !presentOrNull.has_value()) { return std::unexpected {VK_ERROR_UNKNOWN}; }

  indices.graphicsFamily = graphicsOrNull.value();
  indices.presentFamily  = presentOrNull.value();

  return indices;
}

VuQueueFamilyIndices::VuQueueFamilyIndices() = default;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::expected<VuSwapChainSupportDetails, VkResult>
VuSwapChainSupportDetails::make(const VkPhysicalDevice& physicalDevice, const VkSurfaceKHR& surface) noexcept {
  VuSwapChainSupportDetails details;
  try {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, details.formats.data());

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, details.presentModes.data());

  } catch (const std::bad_alloc& badAllocExp) { return std::unexpected {VK_ERROR_OUT_OF_HOST_MEMORY}; } catch (...) {
    return std::unexpected {VK_ERROR_UNKNOWN};
  }

  return details;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
VuPhysicalDevice::VuPhysicalDevice(std::shared_ptr<VuInstance> vuInstance,
                                   const VkSurfaceKHR&         surface,
                                   std::span<const char*>      enabledExtensions) :
    m_vuInstance {std::move(vuInstance)} {

  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(this->m_vuInstance->m_instance, &deviceCount, nullptr);

  if (deviceCount == 0) { throw std::runtime_error("No Vulkan-compatible GPUs found."); }

  std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
  vkEnumeratePhysicalDevices(this->m_vuInstance->m_instance, &deviceCount, physicalDevices.data());

  for (const VkPhysicalDevice& phyDevice : physicalDevices) {
    if (isSupported(phyDevice, surface, enabledExtensions)) {
      this->m_physicalDevice              = phyDevice;
      auto qIndicesOrErr                = VuQueueFamilyIndices::make(phyDevice, surface);
      this->m_indices                     = move_or_THROW(qIndicesOrErr);
      auto swapChainSupportDetailsOrErr = VuSwapChainSupportDetails::make(phyDevice, surface);
      this->m_swapChainSupport            = move_or_THROW(swapChainSupportDetailsOrErr);
      vkGetPhysicalDeviceProperties(phyDevice, &this->m_properties);
      vkGetPhysicalDeviceMemoryProperties(phyDevice, &this->m_memoryProperties);
      vkGetPhysicalDeviceFeatures(phyDevice, &this->m_features);
      return;
    }
  }

  throw VK_ERROR_INITIALIZATION_FAILED;
}
bool
VuPhysicalDevice::isSupported(const VkPhysicalDevice& phyDevice,
                              const VkSurfaceKHR&     surface,
                              std::span<const char*>  enabledExtensions) {

  auto indicesOrErr        = VuQueueFamilyIndices::make(phyDevice, surface);
  bool extensionsSupported = isExtensionsSupported(phyDevice, enabledExtensions);
  bool swapChainAdequate   = false;
  if (extensionsSupported) {
    auto swapChainSupportOrErr = VuSwapChainSupportDetails::make(phyDevice, surface);
    // todo
    THROW_if_unexpected(swapChainSupportOrErr);
    swapChainAdequate = !swapChainSupportOrErr->formats.empty() && !swapChainSupportOrErr->presentModes.empty();
  }
  VkPhysicalDeviceFeatures supportedFeatures {};
  vkGetPhysicalDeviceFeatures(phyDevice, &supportedFeatures);

  return indicesOrErr.has_value() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}
bool
VuPhysicalDevice::isExtensionsSupported(const VkPhysicalDevice& device, std::span<const char*> requestedExtensions) {

  uint32_t extensionCount = 0;
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requested(requestedExtensions.begin(), requestedExtensions.end());

  for (const VkExtensionProperties extension : availableExtensions) {
    requested.erase(extension.extensionName);
  }

  if (requested.empty()) { return true; }

  for (std::string ext : requested) {
    std::cout << "extension not supported: " << ext << std::endl;
  }
  return false;
}
} // namespace Vu