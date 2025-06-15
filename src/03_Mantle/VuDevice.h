#pragma once
#include <set>
#include <vulkan/vulkan_raii.hpp>

#include "VuCommon.h"
#include "VuPhysicalDevice.h"

namespace Vu {

struct VuDeviceCreateFeatureChain {

  vk::PhysicalDeviceRobustness2FeaturesEXT robustness2FeaturesEXT {
      .pNext               = nullptr,
      .robustBufferAccess2 = vk::True,
      .robustImageAccess2  = vk::True,
      .nullDescriptor      = vk::True,
  };

  vk::PhysicalDeviceSynchronization2FeaturesKHR sync2Features {
      .pNext            = &robustness2FeaturesEXT,
      .synchronization2 = VK_TRUE,
  };

  vk::PhysicalDeviceVulkan11Features vk11_features {
      .pNext                              = &sync2Features,
      .storageBuffer16BitAccess           = VK_FALSE,
      .uniformAndStorageBuffer16BitAccess = VK_FALSE,
      .storagePushConstant16              = VK_FALSE,
      .storageInputOutput16               = VK_FALSE,
      .multiview                          = VK_FALSE,
      .multiviewGeometryShader            = VK_FALSE,
      .multiviewTessellationShader        = VK_FALSE,
      .variablePointersStorageBuffer      = VK_FALSE,
      .variablePointers                   = VK_FALSE,
      .protectedMemory                    = VK_FALSE,
      .samplerYcbcrConversion             = VK_FALSE,
      .shaderDrawParameters               = VK_FALSE,
  };

  vk::PhysicalDeviceVulkan12Features vk12_features {
      .samplerMirrorClampToEdge                           = VK_FALSE,
      .drawIndirectCount                                  = VK_FALSE,
      .storageBuffer8BitAccess                            = VK_FALSE,
      .uniformAndStorageBuffer8BitAccess                  = VK_FALSE,
      .storagePushConstant8                               = VK_FALSE,
      .shaderBufferInt64Atomics                           = VK_FALSE,
      .shaderSharedInt64Atomics                           = VK_FALSE,
      .shaderFloat16                                      = VK_FALSE,
      .shaderInt8                                         = VK_FALSE,
      .descriptorIndexing                                 = VK_TRUE,
      .shaderInputAttachmentArrayDynamicIndexing          = VK_TRUE,
      .shaderUniformTexelBufferArrayDynamicIndexing       = VK_TRUE,
      .shaderStorageTexelBufferArrayDynamicIndexing       = VK_TRUE,
      .shaderUniformBufferArrayNonUniformIndexing         = VK_TRUE,
      .shaderSampledImageArrayNonUniformIndexing          = VK_TRUE,
      .shaderStorageBufferArrayNonUniformIndexing         = VK_TRUE,
      .shaderStorageImageArrayNonUniformIndexing          = VK_TRUE,
      .shaderInputAttachmentArrayNonUniformIndexing       = VK_TRUE,
      .shaderUniformTexelBufferArrayNonUniformIndexing    = VK_TRUE,
      .shaderStorageTexelBufferArrayNonUniformIndexing    = VK_TRUE,
      .descriptorBindingUniformBufferUpdateAfterBind      = VK_TRUE,
      .descriptorBindingSampledImageUpdateAfterBind       = VK_TRUE,
      .descriptorBindingStorageImageUpdateAfterBind       = VK_TRUE,
      .descriptorBindingStorageBufferUpdateAfterBind      = VK_TRUE,
      .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
      .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
      .descriptorBindingUpdateUnusedWhilePending          = VK_TRUE,
      .descriptorBindingPartiallyBound                    = VK_TRUE,
      .descriptorBindingVariableDescriptorCount           = VK_FALSE,
      .runtimeDescriptorArray                             = VK_TRUE,
      .samplerFilterMinmax                                = VK_FALSE,
      .scalarBlockLayout                                  = VK_TRUE,
      .imagelessFramebuffer                               = VK_FALSE,
      .uniformBufferStandardLayout                        = VK_FALSE,
      .shaderSubgroupExtendedTypes                        = VK_FALSE,
      .separateDepthStencilLayouts                        = VK_FALSE,
      .hostQueryReset                                     = VK_FALSE,
      .timelineSemaphore                                  = VK_TRUE,
      .bufferDeviceAddress                                = VK_TRUE,
      .bufferDeviceAddressCaptureReplay                   = VK_FALSE,
      .bufferDeviceAddressMultiDevice                     = VK_FALSE,
      .vulkanMemoryModel                                  = VK_FALSE,
      .vulkanMemoryModelDeviceScope                       = VK_FALSE,
      .vulkanMemoryModelAvailabilityVisibilityChains      = VK_FALSE,
      .shaderOutputViewportIndex                          = VK_FALSE,
      .shaderOutputLayer                                  = VK_FALSE,
      .subgroupBroadcastDynamicId                         = VK_FALSE,
  };

  vk::PhysicalDeviceFeatures deviceFeatures {
      .samplerAnisotropy = VK_TRUE,
      .shaderInt64       = VK_TRUE,
  };

  vk::PhysicalDeviceFeatures2 deviceFeatures2 {
      .pNext    = &vk12_features,
      .features = deviceFeatures,
  };
};

struct VuDevice {
  std::shared_ptr<VuPhysicalDevice> vuPhysicalDevice = {};
  vk::raii::Device                  device           = {nullptr};
  vk::raii::Queue                   graphicsQueue    = {nullptr};
  vk::raii::Queue                   presentQueue     = {nullptr};

  static std::expected<VuDevice, vk::Result>
  make(const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
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

  [[nodiscard]] std::expected<vk::raii::PipelineLayout, vk::Result>
  createPipelineLayout(const std::span<vk::DescriptorSetLayout> descriptorSetLayouts,
                       const u32                                pushConstantSizeAsByte) const {

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

  [[nodiscard]] std::expected<vk::raii::DeviceMemory, vk::Result>
  allocateMemory(const vk::MemoryPropertyFlags& memPropFlags, const vk::MemoryRequirements& requirements) const {
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

private:
  VuDevice(const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
           const vk::PhysicalDeviceFeatures2&       featuresChain,
           std::span<const char*>                   enabledExtensions)
      : vuPhysicalDevice {vuPhyDevice} {

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};

    std::set<uint32_t> uniqueQueueFamilies = {vuPhyDevice->indices.graphicsFamily,
                                              vuPhyDevice->indices.presentFamily};

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

  static std::expected<uint32_t, vk::Result>
  findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
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
};

} // namespace Vu
