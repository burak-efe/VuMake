#pragma once

#include "01_InnerCore/zero_optional.h"
#include "02_OuterCore/VuCommon.h"
#include "stb_image.h"

namespace Vu {
struct VuDevice;

struct VuImageCreateInfo {
  uint32_t              width         = 512;
  uint32_t              height        = 512;
  VkFormat              format        = VK_FORMAT_R8G8B8A8_SRGB;
  VkImageTiling         tiling        = VK_IMAGE_TILING_OPTIMAL;
  VkImageUsageFlags     usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
  VkImageAspectFlags    aspectMask    = VK_IMAGE_ASPECT_COLOR_BIT;
};
// #####################################################################################################################
struct VuImage {
  std::shared_ptr<VuDevice> m_vuDevice {nullptr};
  VkDeviceMemory            m_imageMemory {nullptr};
  VkImage                   m_image {nullptr};
  VkImageView               m_imageView {nullptr};
  VuImageCreateInfo         m_lastCreateInfo = {};
  zero_optional<uint32_t>   m_bindlessIndex  = {};

  static void
  loadImageFile(
      const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& out_pixels);
  //--------------------------------------------------------------------------------------------------------------------
  VuImage();

  VuImage(const VuImage&) = delete;

  VuImage&
  operator=(const VuImage&) = delete;

  VuImage(VuImage&& other) noexcept;

  VuImage&
  operator=(VuImage&& other) noexcept;

  ~VuImage();

  SETUP_EXPECTED_WRAPPER(VuImage,
                         (const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo),
                         (vuDevice, createInfo))
private:
  void
  cleanup();
  //--------------------------------------------------------------------------------------------------------------------

  VuImage(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo);
};
} // namespace Vu
