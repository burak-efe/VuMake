#pragma once

#include <cstdint>                  // for uint32_t
#include <filesystem>               // for path
#include "vulkan/vulkan.hpp"

#include "stb_image.h"
#include "08_LangUtils/TypeDefs.h"  // for u32orNull


namespace Vu
{

struct VuImageCreateInfo
{
    uint32_t                width         = 512;
    uint32_t                height        = 512;
    vk::Format              format        = vk::Format::eR8G8B8A8Srgb;
    vk::ImageTiling         tiling        = vk::ImageTiling::eOptimal;
    vk::ImageUsageFlags     usage         = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    vk::MemoryPropertyFlags memProperties = vk::MemoryPropertyFlagBits::eDeviceLocal;
    vk::ImageAspectFlags    aspectMask    = vk::ImageAspectFlagBits::eColor;
};


struct VuImage
{
    vk::Device        device;
    vk::Image         image;
    VuImageCreateInfo lastCreateInfo;
    vk::DeviceMemory  imageMemory;
    vk::ImageView     imageView;
    u32orNull         bindlessIndex = 0;

    void init(vk::Device                                device,
              const vk::PhysicalDeviceMemoryProperties& memProps,
              const VuImageCreateInfo&                  createInfo);

    void uninit();

    static void loadImageFile(const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels,
                              stbi_uc*&                    pixels);

    static void createImage(vk::Device                                device,
                            const vk::PhysicalDeviceMemoryProperties& memProps,
                            uint32_t                                  width,
                            uint32_t                                  height,
                            vk::Format                                format,
                            vk::ImageTiling                           tiling,
                            vk::ImageUsageFlags                       usage,
                            vk::MemoryPropertyFlags                   properties,
                            vk::Image&                                image,
                            vk::DeviceMemory&                         imageMemory);

    static void createImageView(vk::Device device, vk::Format format, vk::Image image, vk::ImageAspectFlags imageAspect,
                                vk::ImageView& outImageView);
};
}
