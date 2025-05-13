// #pragma once
//
// #include <utility>
//
// #include "VuCommon.h"
// #include "VuUtils.h"
//
// namespace Vu {
// struct VuMemoryAllocator {
//   std::shared_ptr<vk::raii::Device>  device;
//   vk::PhysicalDeviceMemoryProperties phyDeviceMemProps;
//
//   VuMemoryAllocator(std::shared_ptr<vk::raii::Device>         device,
//                     const vk::PhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties)
//       : device {std::move(device)}, phyDeviceMemProps {physicalDeviceMemoryProperties} {}
//
//   [[nodiscard]] std::expected<vk::raii::DeviceMemory, vk::Result>
//   allocateMemory(const vk::MemoryPropertyFlags& memPropFlags, const vk::MemoryRequirements2& requirements) const {
//     auto memTypeIndex =
//         findMemoryTypeIndex(phyDeviceMemProps, requirements.memoryRequirements.memoryTypeBits, memPropFlags);
//
//     if (!memTypeIndex) {
//       return std::unexpected {memTypeIndex.error()};
//     }
//
//     vk::MemoryAllocateInfo allocInfo {};
//     allocInfo.allocationSize  = requirements.memoryRequirements.size;
//     allocInfo.memoryTypeIndex = memTypeIndex.value();
//
//     return device->allocateMemory(allocInfo);
//   }
//
//   static std::expected<uint32_t, vk::Result>
//   findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
//                       uint32_t                                  typeFilter,
//                       vk::MemoryPropertyFlags                   requiredProperties) {
//     for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i) {
//       const bool isTypeSuitable = (typeFilter & (1 << i)) != 0;
//       const bool hasRequiredProperties =
//           (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) == requiredProperties;
//
//       if (isTypeSuitable && hasRequiredProperties) {
//         return i;
//       }
//     }
//     return std::unexpected {vk::Result::eErrorOutOfDeviceMemory};
//   }
// };
// } // namespace Vu
