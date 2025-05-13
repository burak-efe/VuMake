#include "VuImage.h"

#include <filesystem> // for path
#include <string>     // for basic_string

#include "stb_image.h"
#include "VuCommon.h"
#include "VuMemoryAllocator.h"
#include "VuUtils.h" // for findMemoryTypeIndex

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

  // todo
  auto res  = vuDevice->device.createImageView(viewInfo).value();
  imageView = std::move(res);
}

void
Vu::VuImage::loadImageFile(
    const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& out_pixels) {
  out_pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
}

// void
// Vu::VuImage::uninit() {
//   vkDestroyImage(device, image, nullptr);
//   vkFreeMemory(device, imageMemory, nullptr);
//   vkDestroyImageView(device, imageView, nullptr);
// }

// void
// Vu::VuImage::createImage(const vk::raii::Device&                   device,
//                          const vk::PhysicalDeviceMemoryProperties& phyDeviceMemProps,
//                          const uint32_t                            width,
//                          const uint32_t                            height,
//                          const vk::Format                          format,
//                          const vk::ImageTiling                     tiling,
//                          const vk::ImageUsageFlags                 usage,
//                          const vk::MemoryPropertyFlags             memPropertyFlags,
//                          vk::raii::Image&                          image,
//                          vk::raii::DeviceMemory&                   imageMemory,
//                          const VuMemoryAllocator&                  allocator) {
//
//   vk::ImageCreateInfo imageCreateInfo {};
//   imageCreateInfo.imageType     = vk::ImageType::e2D;
//   imageCreateInfo.extent.width  = width;
//   imageCreateInfo.extent.height = height;
//   imageCreateInfo.extent.depth  = 1;
//   imageCreateInfo.mipLevels     = 1;
//   imageCreateInfo.arrayLayers   = 1;
//   imageCreateInfo.format        = format;
//   imageCreateInfo.tiling        = tiling;
//   imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
//   imageCreateInfo.usage         = usage;
//   imageCreateInfo.samples       = vk::SampleCountFlagBits::e1;
//   imageCreateInfo.sharingMode   = vk::SharingMode::eExclusive;
//
//   image = device.createImage(imageCreateInfo).value();
//
//   vk::DeviceImageMemoryRequirements devMemoryRequirements {};
//   devMemoryRequirements.pCreateInfo       = &imageCreateInfo;
//   vk::MemoryRequirements2 memRequirements = device.getImageMemoryRequirements(devMemoryRequirements);
//
//   // todo
//   auto memoryOrErr = allocator.allocateMemory(memPropertyFlags, memRequirements);
//   imageMemory      = std::move(memoryOrErr.value());
//
//   device.bindImageMemory2(vk::BindImageMemoryInfo {
//       .image  = image,
//       .memory = imageMemory,
//   });
// }

// void
// Vu::VuImage::createImageView(const vk::raii::Device& device,
//                              vk::Format              format,
//                              vk::raii::Image&        image,
//                              vk::ImageAspectFlags    imageAspect,
//                              vk::raii::ImageView&    outImageView) {
//   vk::ImageViewCreateInfo viewInfo {};
//   viewInfo.image                           = image;
//   viewInfo.viewType                        = vk::ImageViewType::e2D;
//   viewInfo.format                          = format;
//   viewInfo.subresourceRange.aspectMask     = imageAspect;
//   viewInfo.subresourceRange.baseMipLevel   = 0;
//   viewInfo.subresourceRange.levelCount     = 1;
//   viewInfo.subresourceRange.baseArrayLayer = 0;
//   viewInfo.subresourceRange.layerCount     = 1;
//
//   // todo
//   outImageView = device.createImageView(viewInfo).value;
// }
