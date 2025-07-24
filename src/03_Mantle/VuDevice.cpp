#include "VuDevice.h"

#include <set>

#include "../02_OuterCore/VuCommon.h"
#include "VuPhysicalDevice.h"

namespace Vu {
std::expected<VuDevice, vk::Result>
VuDevice::make(const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
               const vk::PhysicalDeviceFeatures2&       requestedFeatureChain,
               std::span<const char*>                   enabledExtensions) {
  try {
    VuDevice outDevice {vuPhyDevice, requestedFeatureChain, enabledExtensions};
    return outDevice;
  } catch (vk::Result res) {
    return std::unexpected {res};
  } //
  catch (...) {

    return std::unexpected {vk::Result::eErrorUnknown};
  }
}
std::expected<vk::raii::PipelineLayout, vk::Result>
VuDevice::createPipelineLayout(const std::span<vk::DescriptorSetLayout> descriptorSetLayouts,
                               const uint32_t                           pushConstantSizeAsByte) const {

  vk::PushConstantRange pcRange {};
  pcRange.stageFlags = vk::ShaderStageFlagBits::eAll;
  pcRange.size       = pushConstantSizeAsByte;

  vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
  pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
  pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges    = &pcRange;

  // todo
  return device.createPipelineLayout(pipelineLayoutInfo);
}
std::expected<vk::raii::DeviceMemory, vk::Result>
VuDevice::allocateMemory(const vk::MemoryPropertyFlags& memPropFlags,
                         const vk::MemoryRequirements&  requirements) const {
  auto memTypeIndex =
      findMemoryTypeIndex(vuPhysicalDevice->memoryProperties, requirements.memoryTypeBits, memPropFlags);

  if (!memTypeIndex) { return std::unexpected {memTypeIndex.error()}; }

  vk::MemoryAllocateFlagsInfo allocFlagsInfo {};
  allocFlagsInfo.flags = vk::MemoryAllocateFlagBits::eDeviceAddress;

  vk::MemoryAllocateInfo allocInfo {};
  allocInfo.pNext           = &allocFlagsInfo;
  allocInfo.allocationSize  = requirements.size;
  allocInfo.memoryTypeIndex = memTypeIndex.value();

  return device.allocateMemory(allocInfo);
}
VuDevice::VuDevice(const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
                   const vk::PhysicalDeviceFeatures2&       featuresChain,
                   std::span<const char*>                   enabledExtensions)
    : vuPhysicalDevice {vuPhyDevice} {

  std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};

  std::set<uint32_t> uniqueQueueFamilies = {vuPhyDevice->indices.graphicsFamily, vuPhyDevice->indices.presentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    vk::DeviceQueueCreateInfo queueCreateInfo {};
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  vk::DeviceCreateInfo createInfo {};
  createInfo.pNext                   = &featuresChain;
  createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.pEnabledFeatures        = nullptr;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
  createInfo.ppEnabledExtensionNames = enabledExtensions.data();

  auto deviceOrErr = vuPhyDevice->physicalDevice.createDevice(createInfo);
  // todo
  throw_if_unexpected(deviceOrErr);

  auto graphicsQueueOrErr = deviceOrErr->getQueue(vuPhyDevice->indices.graphicsFamily, 0);
  auto presentQueueOrErr  = deviceOrErr->getQueue(vuPhyDevice->indices.presentFamily, 0);

  // todo
  throw_if_unexpected(graphicsQueueOrErr);
  throw_if_unexpected(presentQueueOrErr);

  this->device        = std::move(deviceOrErr.value());
  this->graphicsQueue = std::move(graphicsQueueOrErr.value());
  this->presentQueue  = std::move(presentQueueOrErr.value());
}
std::expected<uint32_t, vk::Result>
VuDevice::findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
                              uint32_t                                  typeFilter,
                              vk::MemoryPropertyFlags                   requiredProperties) {
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    const bool isTypeSuitable = (typeFilter & (1 << i)) != 0;
    const bool hasRequiredProperties =
        (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties;

    if (isTypeSuitable && hasRequiredProperties) { return i; }
  }
  return std::unexpected {vk::Result::eErrorOutOfDeviceMemory};
}
} // namespace Vu