#pragma once

#include <cstdint>    // for uint32_t
#include <filesystem> // for path

#include "01_InnerCore/TypeDefs.h" // for u32orNull
#include "stb_image.h"
#include "VuDevice.h"
#include "vulkan/vulkan.hpp"

namespace Vu {

struct VuImageCreateInfo {
  uint32_t                width         = 512;
  uint32_t                height        = 512;
  vk::Format              format        = vk::Format::eR8G8B8A8Srgb;
  vk::ImageTiling         tiling        = vk::ImageTiling::eOptimal;
  vk::ImageUsageFlags     usage         = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
  vk::MemoryPropertyFlags memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
  vk::ImageAspectFlags    aspectMask    = vk::ImageAspectFlagBits::eColor;
};

struct VuImage {
  std::shared_ptr<VuDevice> vuDevice       = {};
  vk::raii::DeviceMemory    imageMemory    = {nullptr};
  vk::raii::Image           image          = {nullptr};
  vk::raii::ImageView       imageView      = {nullptr};
  VuImageCreateInfo         lastCreateInfo = {};
  u32orNull                 bindlessIndex  = {};

  static std::expected<VuImage, vk::Result>
  make(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo) noexcept;

  VuImage() = default;

  static void
  loadImageFile(
      const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& out_pixels);
  private:
  VuImage(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo);
};
} // namespace Vu
