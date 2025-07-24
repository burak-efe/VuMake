#pragma once

#include <expected>
#include <span>
#include <type_traits>

#include "01_InnerCore/TypeDefs.h"
#include "01_InnerCore/zero_optional.h"
#include "02_OuterCore/VuCommon.h"
#include "VuDevice.h"
#include "VuTypes.h"

namespace Vu {
struct VuDevice;

struct VuBufferCreateInfo {
  VuName                  name                  = {"VuBuffer"};
  vk::DeviceSize          sizeInBytes           = {1};
  vk::BufferUsageFlags    vkUsageFlags          = {vk::BufferUsageFlagBits::eStorageBuffer |
                                                   vk::BufferUsageFlagBits::eShaderDeviceAddress};
  vk::MemoryPropertyFlags vkMemoryPropertyFlags = {vk::MemoryPropertyFlagBits::eDeviceLocal |
                                                   vk::MemoryPropertyFlagBits::eHostVisible |
                                                   vk::MemoryPropertyFlagBits::eHostCoherent};
  vk::MemoryAllocateFlags vkMemoryAllocateFlags = {vk::MemoryAllocateFlagBits::eDeviceAddress};
};

struct VuBuffer {
  vk::raii::DeviceMemory deviceMemory  = {nullptr};
  vk::raii::Buffer       buffer        = {nullptr};
  void*                  mapPtr        = {};
  vk::DeviceSize         sizeInBytes   = {};
  zero_optional<u32>     bindlessIndex = {};
  VuName                 name          = {"VuBuffer"};

  VuBuffer() = delete;
  VuBuffer(std::nullptr_t) {}

  static std::expected<VuBuffer, vk::Result>
  make(const VuDevice& vuDevice, const VuBufferCreateInfo& createInfo);

  void
  map();

  void
  unmap();

  [[nodiscard]] vk::DeviceAddress
  getDeviceAddress() const;

  vk::Result
  setData(const void* data, vk::DeviceSize byteSize, vk::DeviceSize offsetInByte = 0) const;

  [[nodiscard]] vk::DeviceSize
  getSizeInBytes() const;

  [[nodiscard]] std::span<byte>
  getMappedSpan(vk::DeviceSize start, vk::DeviceSize sizeInBytes) const;

  static vk::DeviceSize
  calculateAlignedSize(vk::DeviceSize sizeInBytes, vk::DeviceSize alignment);

private:
  VuBuffer(const VuDevice& vuDevice, const VuBufferCreateInfo& createInfo);
};

static_assert(std::is_move_constructible_v<VuBuffer>);
static_assert(std::is_move_assignable_v<VuBuffer>);
} // namespace Vu
