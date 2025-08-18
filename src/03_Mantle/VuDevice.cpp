#include "VuDevice.h"

#include <set>

#include "../02_OuterCore/VuCommon.h"
#include "VuPhysicalDevice.h"

namespace Vu {

std::expected<VkPipelineLayout, VkResult>
VuDevice::createPipelineLayout(const std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                               const uint32_t                         pushConstantSizeAsByte) const {

  VkPushConstantRange pcRange {};
  pcRange.stageFlags = VK_SHADER_STAGE_ALL;
  pcRange.size       = pushConstantSizeAsByte;

  VkPipelineLayoutCreateInfo pipelineLayoutInfo {.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
  pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
  pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges    = &pcRange;

  VkPipelineLayout pipelineLayout {};
  VkResult pipelineRes = vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, NO_ALLOC_CALLBACK, &pipelineLayout);
  RETURN_UNEXPECTED_ON_FAIL(pipelineRes);
  return pipelineLayout;
}
std::expected<VkDeviceMemory, VkResult>
VuDevice::allocateMemory(const VkMemoryPropertyFlags& memPropFlags, const VkMemoryRequirements& requirements) const {
  auto memTypeIndex =
      findMemoryTypeIndex(m_vuPhysicalDevice->m_memoryProperties, requirements.memoryTypeBits, memPropFlags);

  if (!memTypeIndex) { return std::unexpected {memTypeIndex.error()}; }

  VkMemoryAllocateFlagsInfo allocFlagsInfo {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO};
  allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

  VkMemoryAllocateInfo allocInfo {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
  allocInfo.pNext           = &allocFlagsInfo;
  allocInfo.allocationSize  = requirements.size;
  allocInfo.memoryTypeIndex = memTypeIndex.value();

  VkDeviceMemory memory {};
  VkResult       memoryRes = vkAllocateMemory(m_device, &allocInfo, NO_ALLOC_CALLBACK, &memory);
  RETURN_UNEXPECTED_ON_FAIL(memoryRes);
  return memory;
}
VuDevice::VuDevice(const std::shared_ptr<VuPhysicalDevice>& vuPhyDevice,
                   const VkPhysicalDeviceFeatures2&         featuresChain,
                   std::span<const char*>                   enabledExtensions) :
    m_vuPhysicalDevice {vuPhyDevice} {

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos {};

  std::set<uint32_t> uniqueQueueFamilies = {vuPhyDevice->m_indices.graphicsFamily, vuPhyDevice->m_indices.presentFamily};

  float queuePriority = 1.0f;
  for (uint32_t queueFamily : uniqueQueueFamilies) {
    VkDeviceQueueCreateInfo queueCreateInfo {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queueCreateInfo.queueFamilyIndex = queueFamily;
    queueCreateInfo.queueCount       = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;
    queueCreateInfos.push_back(queueCreateInfo);
  }

  VkDeviceCreateInfo createInfo {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
  createInfo.pNext                   = &featuresChain;
  createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
  createInfo.pQueueCreateInfos       = queueCreateInfos.data();
  createInfo.pEnabledFeatures        = nullptr;
  createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
  createInfo.ppEnabledExtensionNames = enabledExtensions.data();

  VkResult deviceRes = vkCreateDevice(vuPhyDevice->m_physicalDevice, &createInfo, NO_ALLOC_CALLBACK, &m_device);
  THROW_if_fail(deviceRes);

  vkGetDeviceQueue(m_device, vuPhyDevice->m_indices.graphicsFamily, 0, &m_graphicsQueue);
  vkGetDeviceQueue(m_device, vuPhyDevice->m_indices.presentFamily, 0, &m_presentQueue);
}
std::expected<uint32_t, VkResult>
VuDevice::findMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& memoryProperties,
                              uint32_t                                typeFilter,
                              VkMemoryPropertyFlags                   requiredProperties) {
  for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
    const bool isTypeSuitable = (typeFilter & (1 << i)) != 0;
    const bool hasRequiredProperties =
        (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties;

    if (isTypeSuitable && hasRequiredProperties) { return i; }
  }
  return std::unexpected {VK_ERROR_OUT_OF_DEVICE_MEMORY};
}
} // namespace Vu