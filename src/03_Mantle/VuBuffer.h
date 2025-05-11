#pragma once

#include <span> // for span

#include "01_InnerCore/TypeDefs.h" // for byte, u32orNull
#include "VuCommon.h"
#include "VuTypes.h" // for VuName

namespace Vu {
struct VuMemoryAllocator;
}

namespace Vu {
struct VuBufferCreateInfo {
  VuName                  name                  = {"VuBuffer"};
  vk::DeviceSize          sizeInBytes           = {1};
  vk::BufferUsageFlags    vkUsageFlags          = {vk::BufferUsageFlagBits::eStorageBuffer |
                                                   vk::BufferUsageFlagBits::eShaderDeviceAddress};
  vk::MemoryAllocateFlags vkMemoryAllocateFlags = {vk::MemoryAllocateFlagBits::eDeviceAddress};
  vk::MemoryPropertyFlags vkMemoryPropertyFlags = {vk::MemoryPropertyFlagBits::eDeviceLocal |
                                                   vk::MemoryPropertyFlagBits::eHostVisible |
                                                   vk::MemoryPropertyFlagBits::eHostCoherent};
};

struct VuBuffer {
  vk::raii::DeviceMemory deviceMemory  = {nullptr};
  vk::raii::Buffer       buffer        = {nullptr};
  void*                  mapPtr        = {};
  vk::DeviceSize         sizeInBytes   = {};
  u32orNull              bindlessIndex = {};
  VuName                 name          = {"VuBuffer"};

  static std::expected<Vu::VuBuffer, vk::Result>
  make(const vk::raii::Device& device, const VuBufferCreateInfo& createInfo, VuMemoryAllocator& allocator);

  void
  map();

  void
  unmap();

  [[nodiscard]] vk::DeviceAddress
  getDeviceAddress() const;

  vk::Result
  setData(const void* data, vk::DeviceSize byteSize, vk::DeviceSize offset = 0) const;

  [[nodiscard]] vk::DeviceSize
  getSizeInBytes() const;

  [[nodiscard]] std::span<byte>
  getMappedSpan(vk::DeviceSize start, vk::DeviceSize sizeInBytes) const;

  static vk::DeviceSize
  alignedSize(vk::DeviceSize sizeInBytes, vk::DeviceSize alignment);
};

static_assert(std::is_move_constructible_v<VuBuffer>);
static_assert(std::is_move_assignable_v<VuBuffer>);
} // namespace Vu
