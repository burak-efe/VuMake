#include "VuBuffer.h"

#include "VuMemoryAllocator.h"
#include "VuUtils.h"

namespace Vu {
std::expected<Vu::VuBuffer, vk::Result>
VuBuffer::make(const std::shared_ptr<VuDevice>& vuDevice, const VuBufferCreateInfo& createInfo) {
  try {
    VuBuffer outBuffer {vuDevice, createInfo};
    return std::move(outBuffer);

  } catch (vk::Result res) {
    return std::unexpected { res };
  } catch (...) {
    return std::unexpected { vk::Result::eErrorUnknown };
  }
}

// VuBuffer::~VuBuffer() {
//   if (mapPtr != nullptr) {
//     unmap();
//   }
// }

// void VuBuffer::uninit()
// {
//     if (mapPtr != nullptr) { unmap(); }
//     vmaDestroyBuffer(vma, buffer, allocation);
// }

void
VuBuffer::map() {
  // todo
  auto res = buffer.getDevice().mapMemory(deviceMemory, 0ull, vk::WholeSize, {}, &mapPtr);
  // vmaMapMemory(vma, allocation, &mapPtr);
}

void
VuBuffer::unmap() {
  buffer.getDevice().unmapMemory(deviceMemory);
  mapPtr = nullptr;
}

vk::DeviceAddress
VuBuffer::getDeviceAddress() const {
  vk::BufferDeviceAddressInfo deviceAddressInfo {};
  deviceAddressInfo.buffer = buffer;

  vk::DeviceAddress address = buffer.getDevice().getBufferAddress(deviceAddressInfo);
  return address;
}

vk::Result
VuBuffer::setData(const void* data, vk::DeviceSize byteSize, vk::DeviceSize offset) const {
  std::memcpy(mapPtr, data, byteSize);
  return vk::Result::eSuccess;
}

vk::DeviceSize
VuBuffer::getSizeInBytes() const {
  return sizeInBytes;
}

std::span<byte>
VuBuffer::getMappedSpan(VkDeviceSize start, VkDeviceSize sizeInBytes) const {
  auto* base = static_cast<std::byte*>(mapPtr);
  return {base + start, sizeInBytes};
}

vk::DeviceSize
VuBuffer::alignedSize(vk::DeviceSize sizeInBytes, vk::DeviceSize alignment) {
  return (sizeInBytes + alignment - 1) & ~(alignment - 1);
}
VuBuffer::VuBuffer(const std::shared_ptr<VuDevice>& vuDevice, const VuBufferCreateInfo& createInfo)
    : vuDevice {vuDevice} {

  this->sizeInBytes = createInfo.sizeInBytes;
  this->name        = createInfo.name;

  vk::BufferCreateInfo bufferCreateInfo;
  bufferCreateInfo.size  = createInfo.sizeInBytes;
  bufferCreateInfo.usage = vk::BufferUsageFlagBits::eShaderDeviceAddress;

  auto bufferOrErr = vuDevice->device.createBuffer(bufferCreateInfo);
  throw_if_unexpected(bufferOrErr);
  this->buffer = std::move(bufferOrErr.value());

  auto memOrNull = vuDevice->allocateMemory(createInfo.vkMemoryPropertyFlags, buffer.getMemoryRequirements());
  throw_if_unexpected(memOrNull);
  this->deviceMemory = std::move(memOrNull.value());

  vk::BindBufferMemoryInfo bindInfo {};
  bindInfo.buffer       = buffer;
  bindInfo.memory       = deviceMemory;
  bindInfo.memoryOffset = 0ull;

  vuDevice->device.bindBufferMemory2(bindInfo);
}
} // namespace Vu
