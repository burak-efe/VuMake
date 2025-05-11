#pragma once
#include <set>
#include <vulkan/vulkan_raii.hpp>

#include "VuCommon.h"
#include "VuPhysicalDevice.h"

namespace Vu {

struct VuDeviceCreateFeatureChain {

  vk::PhysicalDeviceSynchronization2FeaturesKHR sync2Features {.synchronization2 = VK_TRUE};

  vk::PhysicalDeviceVulkan11Features vk11_features {.pNext                              = &sync2Features,
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
                                                    .shaderDrawParameters               = VK_FALSE};

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

struct VuDevice2 {
  vk::raii::Device device        = {nullptr};
  vk::raii::Queue  graphicsQueue = {nullptr};
  vk::raii::Queue  presentQueue  = {nullptr};

  static std::expected<VuDevice2, vk::Result>
  make(const VuPhysicalDevice&            physicalDevice,
       const vk::PhysicalDeviceFeatures2& features,
       std::span<const char*>             enabledExtensions) {

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};

    std::set<uint32_t> uniqueQueueFamilies = {physicalDevice.indices.graphicsFamily.value(),
                                              physicalDevice.indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
      vk::DeviceQueueCreateInfo queueCreateInfo {};
      queueCreateInfo.queueFamilyIndex = queueFamily;
      queueCreateInfo.queueCount       = 1;
      queueCreateInfo.pQueuePriorities = &queuePriority;
      queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::DeviceCreateInfo createInfo {};
    createInfo.pNext                   = &features;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = nullptr;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    auto deviceOrErr = physicalDevice.physicalDevice.createDevice(createInfo);
    // todo
    throw_if_unexpected(deviceOrErr);

    auto graphicsQueueOrErr = deviceOrErr->getQueue(physicalDevice.indices.graphicsFamily.value(), 0);
    auto presentQueueOrErr  = deviceOrErr->getQueue(physicalDevice.indices.presentFamily.value(), 0);

    // todo
    throw_if_unexpected(graphicsQueueOrErr);
    throw_if_unexpected(presentQueueOrErr);

    VuDevice2 outDevice {};
    outDevice.device        = std::move(deviceOrErr.value());
    outDevice.graphicsQueue = std::move(graphicsQueueOrErr.value());
    outDevice.presentQueue  = std::move(presentQueueOrErr.value());
    return outDevice;
  }

  std::expected<vk::raii::PipelineLayout, vk::Result>
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
};

} // namespace Vu