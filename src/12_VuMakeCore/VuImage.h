#pragma once

#include <stb_image.h>
#include <filesystem>

#include "10_Core/VuCommon.h"


namespace Vu
{
    struct VuDevice;

    struct VuImageCreateInfo
    {
        uint32_t              width      = 512;
        uint32_t              height     = 512;
        VkFormat              format     = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageTiling         tiling     = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags     usage      = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkMemoryPropertyFlags memProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageAspectFlags    aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    };

    struct VuImage
    {
        VkDevice          device;
        VkImage           image;
        VuImageCreateInfo lastCreateInfo;
        VkDeviceMemory    imageMemory;
        VkImageView       imageView;

        void init(VkDevice                 device, const VkPhysicalDeviceMemoryProperties& memProps,
                  const VuImageCreateInfo& createInfo);

        void uninit();

        static void loadImageFile(const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels,
                                  stbi_uc*&                    pixels);

        static void createImage(VkDevice                                device,
                                const VkPhysicalDeviceMemoryProperties& memProps,
                                uint32_t                                width,
                                uint32_t                                height,
                                VkFormat                                format,
                                VkImageTiling                           tiling,
                                VkImageUsageFlags                       usage,
                                VkMemoryPropertyFlags                   properties,
                                VkImage&                                image,
                                VkDeviceMemory&                         imageMemory);

        static void createImageView(VkDevice     device, VkFormat format, VkImage image, VkImageAspectFlags imageAspect,
                                    VkImageView& outImageView);
    };
}
