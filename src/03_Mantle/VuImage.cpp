#include "VuImage.h"

#include <filesystem> // for path
#include <string>     // for basic_string

#include "stb_image.h"
#include "VuCommon.h"
#include "VuMemoryAllocator.h"

std::expected<Vu::VuImage, vk::Result>
Vu::VuImage::make(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo) noexcept {
  try {
    VuImage outImage {vuDevice, createInfo};
    return std::move(outImage);
  } catch (vk::Result res) {
    return std::unexpected {res}; //
  } catch (...) {
    return std::unexpected {vk::Result::eErrorUnknown}; //
  }
}
Vu::VuImage::VuImage(const std::shared_ptr<VuDevice>& vuDevice, const VuImageCreateInfo& createInfo)
    : vuDevice {vuDevice}, lastCreateInfo {createInfo} {

  vk::ImageCreateInfo imageCreateInfo {};
  imageCreateInfo.imageType     = vk::ImageType::e2D;
  imageCreateInfo.extent.width  = createInfo.width;
  imageCreateInfo.extent.height = createInfo.height;
  imageCreateInfo.extent.depth  = 1;
  imageCreateInfo.mipLevels     = 1;
  imageCreateInfo.arrayLayers   = 1;
  imageCreateInfo.format        = createInfo.format;
  imageCreateInfo.tiling        = createInfo.tiling;
  imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
  imageCreateInfo.usage         = createInfo.usage;
  imageCreateInfo.samples       = vk::SampleCountFlagBits::e1;
  imageCreateInfo.sharingMode   = vk::SharingMode::eExclusive;
  image                         = vuDevice->device.createImage(imageCreateInfo).value();

  auto memRequirements = image.getMemoryRequirements();

  // todo
  auto memoryOrErr = vuDevice->allocateMemory(createInfo.memProperties, memRequirements);
  throw_if_unexpected(memoryOrErr);
  imageMemory = std::move(memoryOrErr.value());

  vk::BindImageMemoryInfo bindInfo {};
  bindInfo.image  = image;
  bindInfo.memory = imageMemory;

  vuDevice->device.bindImageMemory2(bindInfo);

  vk::ImageViewCreateInfo viewInfo {};
  viewInfo.image                           = image;
  viewInfo.viewType                        = vk::ImageViewType::e2D;
  viewInfo.format                          = createInfo.format;
  viewInfo.subresourceRange.aspectMask     = createInfo.aspectMask;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  imageView = move_or_THROW(vuDevice->device.createImageView(viewInfo));
}

void
Vu::VuImage::loadImageFile(
    const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& out_pixels) {
  out_pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
}
