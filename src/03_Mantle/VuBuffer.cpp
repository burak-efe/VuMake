#include "VuBuffer.h"

#include "VuMemoryAllocator.h"
#include "VuUtils.h"

namespace Vu {
std::expected<Vu::VuBuffer, vk::Result>
VuBuffer::make(const vk::raii::Device& device, const VuBufferCreateInfo& createInfo, VuMemoryAllocator& allocator) {
  Vu::VuBuffer vuBuffer;
  vuBuffer.sizeInBytes = createInfo.sizeInBytes;
  vuBuffer.name        = createInfo.name;

  vk::BufferCreateInfo bufferCreateInfo;
  bufferCreateInfo.size  = createInfo.sizeInBytes;
  bufferCreateInfo.usage = vk::BufferUsageFlagBits::eShaderDeviceAddress;

  auto bufferOrNull = device.createBuffer(bufferCreateInfo);
  if (!bufferOrNull) {
    return std::unexpected {bufferOrNull.error()};
  }

  vk::DeviceBufferMemoryRequirements bufferMemReq {};
  bufferMemReq.pCreateInfo = &bufferCreateInfo;

  vk::MemoryRequirements2 memory_requirements = device.getBufferMemoryRequirements(bufferMemReq);

  auto memOrNull = allocator.allocateMemory(createInfo.vkMemoryPropertyFlags, memory_requirements);
  if (!memOrNull) {
    return std::unexpected {memOrNull.error()};
  }

  vk::BindBufferMemoryInfo bindInfo {};
  bindInfo.buffer       = bufferOrNull.value();
  bindInfo.memory       = memOrNull.value();
  bindInfo.memoryOffset = 0ull;

  device.bindBufferMemory2(bindInfo);

  return std::move(vuBuffer);
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
} // namespace Vu
