#pragma once

#include <iostream>
#include <set>

#include "VuCommon.h"
#include "VuInstance.h"
#include "VuTypes.h"

namespace Vu {

struct VuQueueFamilyIndices {
  std::optional<u32> graphicsFamily {};
  std::optional<u32> presentFamily {};

  bool
  isComplete() const {
    return graphicsFamily.has_value() && presentFamily.has_value();
  }

  // todo make no except
  static VuQueueFamilyIndices
  make(const vk::raii::PhysicalDevice& physDevice, const vk::raii::SurfaceKHR& surface) {
    VuQueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies = physDevice.getQueueFamilyProperties();

    int queueFamilyIndex = 0;

    for (const auto& queue_family : queueFamilies) {
      if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics) { indices.graphicsFamily = queueFamilyIndex; }

      if (physDevice.getSurfaceSupportKHR(queueFamilyIndex, surface)) { indices.presentFamily = queueFamilyIndex; }

      if (indices.isComplete()) { break; }

      queueFamilyIndex++;
    }

    return indices;
  }
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VuSwapChainSupportDetails {
  vk::SurfaceCapabilitiesKHR        capabilities {};
  std::vector<vk::SurfaceFormatKHR> formats {};
  std::vector<vk::PresentModeKHR>   presentModes {};

  static std::expected<VuSwapChainSupportDetails, vk::Result>
  make(const vk::raii::PhysicalDevice& physicalDevice, const vk::raii::SurfaceKHR& surface) noexcept {
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
};
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct VuPhysicalDevice {
  std::shared_ptr<VuInstance>        vuInstance       = {};
  vk::raii::PhysicalDevice           physicalDevice   = {nullptr};
  VuQueueFamilyIndices               indices          = {};
  VuSwapChainSupportDetails          swapChainSupport = {};

  vk::PhysicalDeviceProperties       properties       = {};
  vk::PhysicalDeviceMemoryProperties memoryProperties = {};
  vk::PhysicalDeviceFeatures         features         = {};

  static std::expected<VuPhysicalDevice, vk::Result>
  make(const std::shared_ptr<VuInstance>& vuInstance,
       const vk::raii::SurfaceKHR&        surface,
       std::span<const char*>             enabledExtensions) noexcept {
    try {
      VuPhysicalDevice outVuPhyDevice {vuInstance, surface, enabledExtensions};
      return std::move(outVuPhyDevice);
    } catch (vk::Result res) { return std::unexpected {res}; } catch (...) {
      return std::unexpected {vk::Result::eErrorUnknown};
    }
  }

private:
  VuPhysicalDevice(const std::shared_ptr<VuInstance>& vuInstance,
                   const vk::raii::SurfaceKHR&        surface,
                   std::span<const char*>             enabledExtensions)
      : vuInstance {vuInstance} {

    auto phyDevicesOrErr = vuInstance->instance.enumeratePhysicalDevices();
    // todo
    if (phyDevicesOrErr->empty()) { throw vk::Result::eErrorInitializationFailed; }

    for (const vk::raii::PhysicalDevice& phyDevice : phyDevicesOrErr.value()) {
      if (isSupported(phyDevice, surface, enabledExtensions)) {
        this->physicalDevice   = phyDevice;
        this->indices          = VuQueueFamilyIndices::make(phyDevice, surface);
        // todo
        this->swapChainSupport = *VuSwapChainSupportDetails::make(phyDevice, surface);
        this->properties     = phyDevice.getProperties();
        this->memoryProperties = phyDevice.getMemoryProperties();
        this->features        = phyDevice.getFeatures();
        return;
      }
    }

    throw vk::Result::eErrorInitializationFailed;
  }

  static bool
  isSupported(const vk::raii::PhysicalDevice& phyDevice,
              const vk::raii::SurfaceKHR&     surface,
              std::span<const char*>          enabledExtensions) {

    VuQueueFamilyIndices indices             = VuQueueFamilyIndices::make(phyDevice, surface);
    bool                 extensionsSupported = isExtensionsSupported(phyDevice, enabledExtensions);
    bool                 swapChainAdequate   = false;
    if (extensionsSupported) {
      auto swapChainSupportOrErr = VuSwapChainSupportDetails::make(phyDevice, surface);
      // todo
      throw_if_unexpected(swapChainSupportOrErr);
      swapChainAdequate = !swapChainSupportOrErr->formats.empty() && !swapChainSupportOrErr->presentModes.empty();
    }

    vk::PhysicalDeviceFeatures supportedFeatures = phyDevice.getFeatures();

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
  }

  static bool
  isExtensionsSupported(const vk::raii::PhysicalDevice& device, std::span<const char*> requestedExtensions) {

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
};
} // namespace Vu
