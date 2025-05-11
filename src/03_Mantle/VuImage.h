#pragma once

#include "vulkan/vulkan.hpp"
#include <cstdint>    // for uint32_t
#include <filesystem> // for path

#include "01_InnerCore/TypeDefs.h" // for u32orNull
#include "stb_image.h"

namespace Vu {
struct VuMemoryAllocator;
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
  vk::raii::DeviceMemory imageMemory {nullptr};
  vk::raii::Image        image {nullptr};
  vk::raii::ImageView    imageView {nullptr};
  VuImageCreateInfo      lastCreateInfo {};
  u32orNull              bindlessIndex {};

  void
  init(const vk::raii::Device&                   device,
       const vk::PhysicalDeviceMemoryProperties& memProps,
       const VuImageCreateInfo&                  createInfo,
       const VuMemoryAllocator&                  allocator);

  // void
  // uninit();

  static void
  loadImageFile(const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels);

  // static void
  // createImage(const vk::raii::Device&                   device,
  //             const vk::PhysicalDeviceMemoryProperties& memProps,
  //             uint32_t                                  width,
  //             uint32_t                                  height,
  //             vk::Format                                format,
  //             vk::ImageTiling                           tiling,
  //             vk::ImageUsageFlags                       usage,
  //             vk::MemoryPropertyFlags                   properties,
  //             vk::raii::Image&                          image,
  //             vk::raii::DeviceMemory&                   imageMemory,
  //             const VuMemoryAllocator&                  allocator);
  //
  // static void
  // createImageView(const vk::raii::Device& device,
  //                 vk::Format              format,
  //                 vk::raii::Image&        image,
  //                 vk::ImageAspectFlags    imageAspect,
  //                 vk::raii::ImageView&    outImageView);
};
} // namespace Vu
