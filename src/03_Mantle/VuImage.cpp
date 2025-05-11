#include "VuImage.h"

#include <filesystem> // for path
#include <stdexcept>  // for runtime_error
#include <string>     // for basic_string

#include "VuCommon.h"
#include "VuMemoryAllocator.h"
#include "stb_image.h"

#include "02_OuterCore/Common.h" // for vk::Check
#include "VuUtils.h"             // for findMemoryTypeIndex

void
Vu::VuImage::init(const vk::raii::Device&                   device,
                  const vk::PhysicalDeviceMemoryProperties& memProps,
                  const VuImageCreateInfo&                  createInfo,
                  const VuMemoryAllocator&                  allocator) {

  lastCreateInfo                                 = createInfo;
  const uint32_t                width            = createInfo.width;
  const uint32_t                height           = createInfo.height;
  const vk::Format              format           = createInfo.format;
  const vk::ImageTiling         tiling           = createInfo.tiling;
  const vk::ImageUsageFlags     usage            = createInfo.usage;
  const vk::MemoryPropertyFlags memPropertyFlags = createInfo.memProperties;

  vk::ImageCreateInfo imageCreateInfo {};
  imageCreateInfo.imageType     = vk::ImageType::e2D;
  imageCreateInfo.extent.width  = width;
  imageCreateInfo.extent.height = height;
  imageCreateInfo.extent.depth  = 1;
  imageCreateInfo.mipLevels     = 1;
  imageCreateInfo.arrayLayers   = 1;
  imageCreateInfo.format        = format;
  imageCreateInfo.tiling        = tiling;
  imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
  imageCreateInfo.usage         = usage;
  imageCreateInfo.samples       = vk::SampleCountFlagBits::e1;
  imageCreateInfo.sharingMode   = vk::SharingMode::eExclusive;
  image                         = device.createImage(imageCreateInfo).value();

  vk::DeviceImageMemoryRequirements devMemoryRequirements {};
  devMemoryRequirements.pCreateInfo = &imageCreateInfo;

  vk::MemoryRequirements2 memRequirements = device.getImageMemoryRequirements(devMemoryRequirements);
  // todo
  auto memoryOrErr                        = allocator.allocateMemory(memPropertyFlags, memRequirements);
  imageMemory                             = std::move(memoryOrErr.value());

  vk::BindImageMemoryInfo bindInfo {};
  bindInfo.image  = image;
  bindInfo.memory = imageMemory;

  device.bindImageMemory2(bindInfo);

  vk::Format           inlined_format = createInfo.format;
  vk::ImageAspectFlags imageAspect    = createInfo.aspectMask;

  vk::ImageViewCreateInfo viewInfo {};
  viewInfo.image                           = image;
  viewInfo.viewType                        = vk::ImageViewType::e2D;
  viewInfo.format                          = inlined_format;
  viewInfo.subresourceRange.aspectMask     = imageAspect;
  viewInfo.subresourceRange.baseMipLevel   = 0;
  viewInfo.subresourceRange.levelCount     = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount     = 1;

  // todo
  imageView = device.createImageView(viewInfo).value();
}

void
Vu::VuImage::loadImageFile(
    const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels) {
  pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
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
