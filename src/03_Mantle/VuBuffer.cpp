#include "VuBuffer.h"

#include <cstddef>
#include <cstring>
#include <exception>
#include <utility>

#include "../02_OuterCore/VuCommon.h"
#include "03_Mantle/VuDevice.h"

namespace Vu {
std::expected<Vu::VuBuffer, vk::Result> VuBuffer::make(const VuDevice& vuDevice, const VuBufferCreateInfo& createInfo) {
  try {
    VuBuffer outBuffer {vuDevice, createInfo};
    return std::move(outBuffer);

  } catch (vk::Result res) { return std::unexpected {res}; } catch (...) {
    return std::unexpected {vk::Result::eErrorUnknown};
  }
}

void VuBuffer::map() {
  // todo
  auto res = buffer.getDevice().mapMemory(deviceMemory, 0ull, vk::WholeSize, {}, &mapPtr);
  if (res != vk::Result::eSuccess) { throw std::bad_exception(); }
}

void VuBuffer::unmap() {
  buffer.getDevice().unmapMemory(deviceMemory);
  mapPtr = nullptr;
}

vk::DeviceAddress VuBuffer::getDeviceAddress() const {
  vk::BufferDeviceAddressInfo deviceAddressInfo {};
  deviceAddressInfo.buffer = buffer;

  vk::DeviceAddress address = buffer.getDevice().getBufferAddress(deviceAddressInfo);
  return address;
}

vk::Result VuBuffer::setData(const void* data, vk::DeviceSize byteSize, vk::DeviceSize offsetInByte) const {
  if (!mapPtr || !data || byteSize == 0) { return vk::Result::eErrorMemoryMapFailed; }
  auto* dst = static_cast<std::byte*>(mapPtr) + offsetInByte;
  std::memcpy(dst, data, static_cast<size_t>(byteSize));

  return vk::Result::eSuccess;
}

vk::DeviceSize VuBuffer::getSizeInBytes() const { return sizeInBytes; }

std::span<byte> VuBuffer::getMappedSpan(VkDeviceSize start, VkDeviceSize sizeInBytes) const {
  auto* base = static_cast<std::byte*>(mapPtr);
  return {base + start, sizeInBytes};
}

vk::DeviceSize VuBuffer::alignedSize(vk::DeviceSize sizeInBytes, vk::DeviceSize alignment) {
  return (sizeInBytes + alignment - 1) & ~(alignment - 1);
}
VuBuffer::VuBuffer(const VuDevice& vuDevice, const VuBufferCreateInfo& createInfo) {

  this->sizeInBytes = createInfo.sizeInBytes;
  this->name        = createInfo.name;

  vk::BufferCreateInfo bufferCreateInfo;
  bufferCreateInfo.size        = createInfo.sizeInBytes;
  bufferCreateInfo.usage       = createInfo.vkUsageFlags;
  bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

  auto bufferOrErr = vuDevice.device.createBuffer(bufferCreateInfo);
  throw_if_unexpected(bufferOrErr);
  this->buffer = std::move(bufferOrErr.value());

  auto memOrNull = vuDevice.allocateMemory(createInfo.vkMemoryPropertyFlags, buffer.getMemoryRequirements());
  throw_if_unexpected(memOrNull);
  this->deviceMemory = std::move(memOrNull.value());

  vk::BindBufferMemoryInfo bindInfo {};
  bindInfo.buffer       = buffer;
  bindInfo.memory       = deviceMemory;
  bindInfo.memoryOffset = 0ull;

  vuDevice.device.bindBufferMemory2(bindInfo);
}
} // namespace Vu
