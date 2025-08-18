#include "VuBuffer.h"

#include <cstddef>
#include <cstring>
#include <exception>
#include <utility>

#include "02_OuterCore/VuCommon.h"
#include "03_Mantle/VuDevice.h"

namespace Vu {

VkResult
VuBuffer::map() {
  return vkMapMemory(m_vuDevice->m_device, m_deviceMemory, MakeVkOffset(0), VK_WHOLE_SIZE, ZERO_FLAG, &m_mapPtr);
}

void
VuBuffer::unmap() {
  vkUnmapMemory(m_vuDevice->m_device, m_deviceMemory);
  m_mapPtr = nullptr;
}

VkDeviceAddress
VuBuffer::getDeviceAddress() const {
  VkBufferDeviceAddressInfo deviceAddressInfo {.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
  deviceAddressInfo.buffer = m_buffer;
  return vkGetBufferDeviceAddress(m_vuDevice->m_device, &deviceAddressInfo);
}

VkResult
VuBuffer::setData(const void* data, VkDeviceSize byteSize, VkDeviceSize offsetInByte) const {
  if (!m_mapPtr || !data || byteSize == 0) { return VK_ERROR_MEMORY_MAP_FAILED; }
  auto* dst = static_cast<std::byte*>(m_mapPtr) + offsetInByte;
  std::memcpy(dst, data, static_cast<size_t>(byteSize));

  return VK_SUCCESS;
}

VkDeviceSize
VuBuffer::getSizeInBytes() const {
  return m_sizeInBytes;
}

std::span<byte>
VuBuffer::getMappedSpan(VkDeviceSize start, VkDeviceSize sizeInBytes) const {
  auto* base = static_cast<std::byte*>(m_mapPtr);
  return {base + start, sizeInBytes};
}

VkDeviceSize
VuBuffer::calculateAlignedSize(VkDeviceSize sizeInBytes, VkDeviceSize alignment) {
  return (sizeInBytes + alignment - 1) & ~(alignment - 1);
}
VuBuffer::VuBuffer(std::shared_ptr<VuDevice> vuDevice, const VuBufferCreateInfo& createInfo) {

  this->m_vuDevice    = vuDevice;
  this->m_sizeInBytes = createInfo.sizeInBytes;
  this->m_name        = createInfo.name;

  VkBufferCreateInfo bufferCreateInfo {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bufferCreateInfo.size        = createInfo.sizeInBytes;
  bufferCreateInfo.usage       = createInfo.vkUsageFlags;
  bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult bufferRes = vkCreateBuffer(vuDevice->m_device, &bufferCreateInfo, NO_ALLOC_CALLBACK, &m_buffer);
  THROW_if_fail(bufferRes);

  VkMemoryRequirements memoryRequirements {};
  vkGetBufferMemoryRequirements(vuDevice->m_device, m_buffer, &memoryRequirements);

  auto memoryOrErr = vuDevice->allocateMemory(createInfo.vkMemoryPropertyFlags, memoryRequirements);
  THROW_if_unexpected(memoryOrErr);
  this->m_deviceMemory = std::move(memoryOrErr.value());

  VkResult bindRes = vkBindBufferMemory(vuDevice->m_device, m_buffer, m_deviceMemory, MakeVkOffset(0));
  THROW_if_fail(bindRes);
}
} // namespace Vu
