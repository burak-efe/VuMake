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
  VuName             name         = {"VuBuffer"};
  VkDeviceSize       sizeInBytes  = {1};
  VkBufferUsageFlags vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

  VkMemoryPropertyFlags vkMemoryPropertyFlags =
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  VkMemoryAllocateFlags vkMemoryAllocateFlags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
};
// #####################################################################################################################
struct VuBuffer {
  std::shared_ptr<VuDevice> m_vuDevice {nullptr};
  VkDeviceMemory            m_deviceMemory {nullptr};
  VkBuffer                  m_buffer {nullptr};
  void*                     m_mapPtr {};
  VkDeviceSize              m_sizeInBytes {};
  VuName                    m_name {"VuBuffer"};
  zero_optional<u32>        m_bindlessIndex {};

  SETUP_EXPECTED_WRAPPER(VuBuffer,
                         (std::shared_ptr<VuDevice> vuDevice, const VuBufferCreateInfo& createInfo),
                         (vuDevice, createInfo))
public:
  VuBuffer()                = default;
  VuBuffer(const VuBuffer&) = delete;
  VuBuffer&
  operator=(const VuBuffer&) = delete;

  VuBuffer(VuBuffer&& other) noexcept :
      m_vuDevice(std::move(other.m_vuDevice)),
      m_deviceMemory(other.m_deviceMemory),
      m_buffer(other.m_buffer),
      m_mapPtr(other.m_mapPtr),
      m_sizeInBytes(other.m_sizeInBytes),
      m_name(std::move(other.m_name)),
      m_bindlessIndex(other.m_bindlessIndex) {
    other.m_deviceMemory  = VK_NULL_HANDLE;
    other.m_buffer        = VK_NULL_HANDLE;
    other.m_mapPtr        = nullptr;
    other.m_sizeInBytes   = 0;
    other.m_bindlessIndex = zero_optional<u32> {};
  }

  VuBuffer&
  operator=(VuBuffer&& other) noexcept {
    if (this != &other) {
      cleanup();
      m_vuDevice      = std::move(other.m_vuDevice);
      m_deviceMemory  = other.m_deviceMemory;
      m_buffer        = other.m_buffer;
      m_mapPtr        = other.m_mapPtr;
      m_sizeInBytes   = other.m_sizeInBytes;
      m_name          = std::move(other.m_name);
      m_bindlessIndex = other.m_bindlessIndex;

      other.m_deviceMemory  = VK_NULL_HANDLE;
      other.m_buffer        = VK_NULL_HANDLE;
      other.m_mapPtr        = nullptr;
      other.m_sizeInBytes   = 0;
      other.m_bindlessIndex = zero_optional<u32> {};
    }
    return *this;
  }

  ~VuBuffer() { cleanup(); }

private:
  void
  cleanup() {
    if (m_mapPtr) {
      vkUnmapMemory(m_vuDevice->m_device, m_deviceMemory);
      m_mapPtr = nullptr;
    }
    if (m_deviceMemory != VK_NULL_HANDLE) {
      vkFreeMemory(m_vuDevice->m_device, m_deviceMemory, nullptr);
      m_deviceMemory = VK_NULL_HANDLE;
    }
    if (m_buffer != VK_NULL_HANDLE) {
      vkDestroyBuffer(m_vuDevice->m_device, m_buffer, nullptr);
      m_buffer = VK_NULL_HANDLE;
    }
    m_vuDevice.reset();
  }

  VuBuffer(std::shared_ptr<VuDevice> vuDevice, const VuBufferCreateInfo& createInfo);

public:
  [[nodiscard]] VkResult
  map();

  void
  unmap();

  [[nodiscard]] VkDeviceAddress
  getDeviceAddress() const;

  VkResult
  setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offsetInByte = 0) const;

  [[nodiscard]] VkDeviceSize
  getSizeInBytes() const;

  [[nodiscard]] std::span<byte>
  getMappedSpan(VkDeviceSize start, VkDeviceSize sizeInBytes) const;

  static VkDeviceSize
  calculateAlignedSize(VkDeviceSize sizeInBytes, VkDeviceSize alignment);
};

static_assert(std::is_move_constructible_v<VuBuffer>);
static_assert(std::is_move_assignable_v<VuBuffer>);
} // namespace Vu
