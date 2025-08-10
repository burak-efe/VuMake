#pragma once

#include "../02_OuterCore/VuCommon.h"

namespace vk {
class DescriptorSetLayout;
} // namespace vk

namespace Vu {
struct VuPhysicalDevice;

struct VuDeviceCreateFeatureChain {

  // VkPhysicalDeviceRobustness2FeaturesEXT robustness2FeaturesEXT {
  //     .sType               = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
  //     .pNext               = nullptr,
  //     .robustBufferAccess2 = VK_TRUE,
  //     .robustImageAccess2  = VK_TRUE,
  //     .nullDescriptor      = VK_TRUE,
  // };
  //
  // VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features {
  //     .sType            = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
  //     .pNext            = &robustness2FeaturesEXT,
  //     .synchronization2 = VK_TRUE,
  // };

  VkPhysicalDeviceVulkan11Features vk11_features {
      .sType                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
      .pNext                              = nullptr,
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
      .shaderDrawParameters               = VK_TRUE,
  };

  VkPhysicalDeviceVulkan12Features vk12_features {
      .sType                                              = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
      .pNext                                              = &vk11_features,
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

  VkPhysicalDeviceFeatures deviceFeatures {
      .samplerAnisotropy = VK_TRUE,
      .shaderInt64       = VK_TRUE,
  };

  VkPhysicalDeviceFeatures2 deviceFeatures2 {
      .sType    = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
      .pNext    = &vk12_features,
      .features = deviceFeatures,
  };
};

// #####################################################################################################################

struct VuDevice {
  std::shared_ptr<VuPhysicalDevice> m_vuPhysicalDevice {nullptr};
  VkDevice                          m_device {nullptr};
  VkQueue                           m_graphicsQueue {nullptr};
  VkQueue                           m_presentQueue {nullptr};

  [[nodiscard]] std::expected<VkPipelineLayout, VkResult>
  createPipelineLayout(std::span<VkDescriptorSetLayout> descriptorSetLayouts, uint32_t pushConstantSizeAsByte) const;

  [[nodiscard]] std::expected<VkDeviceMemory, VkResult>
  allocateMemory(const VkMemoryPropertyFlags& memPropFlags, const VkMemoryRequirements& requirements) const;

  //--------------------------------------------------------------------------------------------------------------------
  SETUP_EXPECTED_WRAPPER(VuDevice,
                         (const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
                          const VkPhysicalDeviceFeatures2&         featuresChain,
                          std::span<const char*>                   enabledExtensions),
                         (vuPhyDevice, featuresChain, enabledExtensions))
  VuDevice()                = default;
  VuDevice(const VuDevice&) = delete;
  VuDevice&
  operator=(const VuDevice&) = delete;

  VuDevice(VuDevice&& other) noexcept :
      m_vuPhysicalDevice(std::move(other.m_vuPhysicalDevice)),
      m_device(other.m_device),
      m_graphicsQueue(other.m_graphicsQueue),
      m_presentQueue(other.m_presentQueue) {
    other.m_device        = VK_NULL_HANDLE;
    other.m_graphicsQueue = VK_NULL_HANDLE;
    other.m_presentQueue  = VK_NULL_HANDLE;
  }

  VuDevice&
  operator=(VuDevice&& other) noexcept {
    if (this != &other) {
      cleanup();
      m_vuPhysicalDevice    = std::move(other.m_vuPhysicalDevice);
      m_device              = other.m_device;
      m_graphicsQueue       = other.m_graphicsQueue;
      m_presentQueue        = other.m_presentQueue;
      other.m_device        = VK_NULL_HANDLE;
      other.m_graphicsQueue = VK_NULL_HANDLE;
      other.m_presentQueue  = VK_NULL_HANDLE;
    }
    return *this;
  }

  ~VuDevice() { cleanup(); }

private:
  void
  cleanup() {
    if (m_device != VK_NULL_HANDLE) {
      vkDestroyDevice(m_device, nullptr);
      m_device = VK_NULL_HANDLE;
      m_graphicsQueue = VK_NULL_HANDLE;
      m_presentQueue  = VK_NULL_HANDLE;
      m_vuPhysicalDevice.reset();
    }
  }

  VuDevice(const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
           const VkPhysicalDeviceFeatures2&         featuresChain,
           std::span<const char*>                   enabledExtensions);

  static std::expected<uint32_t, VkResult>
  findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties,
                      uint32_t                                typeFilter,
                      VkMemoryPropertyFlags                   requiredProperties);
};

} // namespace Vu
