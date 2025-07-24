#pragma once

#include "01_InnerCore/TypeDefs.h"
#include "01_InnerCore/zero_optional.h"
#include "02_OuterCore/VuCommon.h"
#include "stb_image.h"

namespace Vu {
struct VuDevice;

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
  std::shared_ptr<VuDevice> vuDevice       = {nullptr};
  vk::raii::DeviceMemory    imageMemory    = {nullptr};
  vk::raii::Image           image          = {nullptr};
  vk::raii::ImageView       imageView      = {nullptr};
  VuImageCreateInfo         lastCreateInfo = {};
  zero_optional<u32>        bindlessIndex  = {};

  VuImage() = delete;
  VuImage(std::nullptr_t) {}

  static std::expected<VuImage, vk::Result>
  make(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo) noexcept;

  static void
  loadImageFile(
      const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& out_pixels);

private:
  VuImage(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo);
};

// static_assert(std::is_default_constructible_v<VuImage> == false);
// static_assert(std::is_copy_assignable_v<VuImage> == false);
// static_assert(std::is_copy_constructible_v<VuImage> == false);
//
// static_assert(std::is_move_constructible_v<VuImage> == true);
// static_assert(std::is_move_assignable_v<VuImage> == true);
// static_assert(std::is_destructible_v<VuImage> == true);
//
// static_assert(std::is_nothrow_destructible_v<VuImage> == true);
// static_assert(std::is_nothrow_move_assignable_v<VuImage> == true);
// static_assert(std::is_nothrow_move_constructible_v<VuImage> == true);
//
// static_assert(std::is_constructible_v<VuImage, std::nullptr_t>, "must be constructible from nullptr");

} // namespace Vu
