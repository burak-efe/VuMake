#include "VuImage.h"

#include "../02_OuterCore/VuCommon.h"
#include "VuDevice.h"

Vu::VuImage::VuImage(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo) :
    m_vuDevice {vuDevice},
    m_lastCreateInfo {createInfo} {

  VkImageCreateInfo imageCreateInfo {};
  imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
  imageCreateInfo.extent.width  = createInfo.width;
  imageCreateInfo.extent.height = createInfo.height;
  imageCreateInfo.extent.depth  = 1;
  imageCreateInfo.mipLevels     = 1;
  imageCreateInfo.arrayLayers   = 1;
  imageCreateInfo.format        = createInfo.format;
  imageCreateInfo.tiling        = createInfo.tiling;
  imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageCreateInfo.usage         = createInfo.usage;
  imageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

  VkResult imageRes = vkCreateImage(vuDevice->m_device, &imageCreateInfo, nullptr, &m_image);
  THROW_if_fail(imageRes);

  VkMemoryRequirements memRequirements;
  vkGetImageMemoryRequirements(vuDevice->m_device, m_image, &memRequirements);

  auto memoryOrErr = vuDevice->allocateMemory(createInfo.memProperties, memRequirements);
  THROW_if_unexpected(memoryOrErr);
  m_imageMemory = std::move(memoryOrErr.value());

  VkResult bindRes = vkBindImageMemory(vuDevice->m_device, m_image, m_imageMemory, MakeVkOffset(0));
  THROW_if_fail(bindRes);

  VkImageViewCreateInfo viewInfo {};
  viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image                           = m_image;
  viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format                          = createInfo.format;
  viewInfo.subresourceRange.aspectMask     = createInfo.aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  VkResult viewRes = vkCreateImageView(vuDevice->m_device, &viewInfo, NO_ALLOC_CALLBACK, &m_imageView);
  THROW_if_fail(viewRes);
}

void
Vu::VuImage::loadImageFile(
    const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& out_pixels) {
  out_pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
}

Vu::VuImage::VuImage() = default;

Vu::VuImage::VuImage(VuImage&& other) noexcept :
    m_vuDevice(std::move(other.m_vuDevice)),
    m_imageMemory(other.m_imageMemory),
    m_image(other.m_image),
    m_imageView(other.m_imageView),
    m_lastCreateInfo(std::move(other.m_lastCreateInfo)),
    m_bindlessIndex(other.m_bindlessIndex) {
  other.m_imageMemory   = VK_NULL_HANDLE;
  other.m_image         = VK_NULL_HANDLE;
  other.m_imageView     = VK_NULL_HANDLE;
  other.m_bindlessIndex = zero_optional<uint32_t> {};
}

Vu::VuImage&
Vu::VuImage::operator=(VuImage&& other) noexcept {
  if (this != &other) {
    cleanup();
    m_vuDevice       = std::move(other.m_vuDevice);
    m_imageMemory    = other.m_imageMemory;
    m_image          = other.m_image;
    m_imageView      = other.m_imageView;
    m_lastCreateInfo = std::move(other.m_lastCreateInfo);
    m_bindlessIndex  = other.m_bindlessIndex;

    other.m_imageMemory   = VK_NULL_HANDLE;
    other.m_image         = VK_NULL_HANDLE;
    other.m_imageView     = VK_NULL_HANDLE;
    other.m_bindlessIndex = zero_optional<uint32_t> {};
  }
  return *this;
}

Vu::VuImage::~VuImage() { cleanup(); }

void
Vu::VuImage::cleanup() {
  if (m_imageView != VK_NULL_HANDLE) {
    vkDestroyImageView(m_vuDevice->m_device, m_imageView, nullptr);
    m_imageView = VK_NULL_HANDLE;
  }
  if (m_image != VK_NULL_HANDLE) {
    vkDestroyImage(m_vuDevice->m_device, m_image, nullptr);
    m_image = VK_NULL_HANDLE;
  }
  if (m_imageMemory != VK_NULL_HANDLE) {
    vkFreeMemory(m_vuDevice->m_device, m_imageMemory, nullptr);
    m_imageMemory = VK_NULL_HANDLE;
  }
  m_vuDevice.reset();
}
