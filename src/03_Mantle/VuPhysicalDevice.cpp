
#include "VuPhysicalDevice.h"

#include <set>

#include "../02_OuterCore/VuCommon.h"
#include "VuInstance.h"

namespace Vu {
VuQueueFamilyIndices::VuQueueFamilyIndices(std::nullptr_t) {}
std::expected<VuQueueFamilyIndices, vk::Result>
VuQueueFamilyIndices::make(const vk::raii::PhysicalDevice& physDevice, const vk::raii::SurfaceKHR& surface) noexcept {
  VuQueueFamilyIndices                   indices;
  std::vector<vk::QueueFamilyProperties> queueFamilies    = physDevice.getQueueFamilyProperties();
  int                                    queueFamilyIndex = 0;

  std::optional<uint32_t> graphicsOrNull = {std::nullopt};
  std::optional<uint32_t> presentOrNull  = {std::nullopt};

  for (const auto& queueFamily : queueFamilies) {

    if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) { graphicsOrNull = queueFamilyIndex; }

    if (physDevice.getSurfaceSupportKHR(queueFamilyIndex, surface)) { presentOrNull = queueFamilyIndex; }

    if (graphicsOrNull.has_value() && presentOrNull.has_value()) { break; }

    queueFamilyIndex++;
  }

  if (!graphicsOrNull.has_value() || !presentOrNull.has_value()) { return std::unexpected {vk::Result::eErrorUnknown}; }

  indices.graphicsFamily = graphicsOrNull.value();
  indices.presentFamily  = presentOrNull.value();

  return indices;
}

VuQueueFamilyIndices::VuQueueFamilyIndices() = default;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::expected<VuSwapChainSupportDetails, vk::Result>
VuSwapChainSupportDetails::make(const vk::raii::PhysicalDevice& physicalDevice,
                                const vk::raii::SurfaceKHR&     surface) noexcept {
  VuSwapChainSupportDetails details;
  try {
    details.capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface);
    details.formats      = physicalDevice.getSurfaceFormatsKHR(surface);
    details.presentModes = physicalDevice.getSurfacePresentModesKHR(surface);
  } catch (const std::bad_alloc& badAllocExp) {
    return std::unexpected {vk::Result::eErrorOutOfHostMemory};
  } catch (...) { return std::unexpected {vk::Result::eErrorUnknown}; }

  return details;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::expected<VuPhysicalDevice, vk::Result>
VuPhysicalDevice::make(const std::shared_ptr<VuInstance>& vuInstance,
                       const vk::raii::SurfaceKHR&        surface,
                       std::span<const char*>             enabledExtensions) noexcept {
  try {
    VuPhysicalDevice outVuPhyDevice {vuInstance, surface, enabledExtensions};
    return std::move(outVuPhyDevice);
  } catch (vk::Result res) { return std::unexpected {res}; } catch (...) {
    return std::unexpected {vk::Result::eErrorUnknown};
  }
}
VuPhysicalDevice::VuPhysicalDevice(const std::shared_ptr<VuInstance>& vuInstance,
                                   const vk::raii::SurfaceKHR&        surface,
                                   std::span<const char*>             enabledExtensions)
    : vuInstance {vuInstance} {

  auto phyDevicesOrErr = vuInstance->instance.enumeratePhysicalDevices();
  // todo
  if (phyDevicesOrErr->empty()) { throw vk::Result::eErrorInitializationFailed; }

  for (const vk::raii::PhysicalDevice& phyDevice : phyDevicesOrErr.value()) {
    if (isSupported(phyDevice, surface, enabledExtensions)) {
      this->physicalDevice   = phyDevice;
      auto qIndicesOrErr     = VuQueueFamilyIndices::make(phyDevice, surface);
      this->indices          = moveOrTHROW(qIndicesOrErr);
      // todo
      this->swapChainSupport = *VuSwapChainSupportDetails::make(phyDevice, surface);
      this->properties       = phyDevice.getProperties();
      this->memoryProperties = phyDevice.getMemoryProperties();
      this->features         = phyDevice.getFeatures();
      return;
    }
  }

  throw vk::Result::eErrorInitializationFailed;
}
bool
VuPhysicalDevice::isSupported(const vk::raii::PhysicalDevice& phyDevice,
                              const vk::raii::SurfaceKHR&     surface,
                              std::span<const char*>          enabledExtensions) {

  auto indicesOrErr        = VuQueueFamilyIndices::make(phyDevice, surface);
  bool extensionsSupported = isExtensionsSupported(phyDevice, enabledExtensions);
  bool swapChainAdequate   = false;
  if (extensionsSupported) {
    auto swapChainSupportOrErr = VuSwapChainSupportDetails::make(phyDevice, surface);
    // todo
    throw_if_unexpected(swapChainSupportOrErr);
    swapChainAdequate = !swapChainSupportOrErr->formats.empty() && !swapChainSupportOrErr->presentModes.empty();
  }

  vk::PhysicalDeviceFeatures supportedFeatures = phyDevice.getFeatures();

  return indicesOrErr.has_value() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}
bool
VuPhysicalDevice::isExtensionsSupported(const vk::raii::PhysicalDevice& device,
                                        std::span<const char*>          requestedExtensions) {

  std::vector<vk::ExtensionProperties> availableExtensions = device.enumerateDeviceExtensionProperties();

  std::set<std::string> requested(requestedExtensions.begin(), requestedExtensions.end());

  for (const vk::ExtensionProperties extension : availableExtensions) {
    requested.erase(extension.extensionName);
  }

  if (requested.empty()) { return true; }

  for (std::string ext : requested) {
    std::cout << "extension not supported: " << ext << std::endl;
  }
  return false;
}
} // namespace Vu