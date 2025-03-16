#pragma once

#include "Common.h"


namespace Vu
{
    struct VuDevice;

    struct VuTextureCreateInfo
    {
        VkDevice              device;
        VkPhysicalDevice      physicalDevice;
        uint32_t              width;
        uint32_t              height;
        VkFormat              format     = VK_FORMAT_R8G8B8A8_SRGB;
        VkImageTiling         tiling     = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags     usage      = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        VkMemoryPropertyFlags properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        VkImageAspectFlags    aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    };


    struct VuImage
    {
        VuTextureCreateInfo lastCreateInfo;
        VkImage             image;
        VkDeviceMemory      imageMemory;
        VkImageView         imageView;

        void init(const VuTextureCreateInfo& createInfo);

        void initFromAsset(VuDevice& vuDevice, const Path& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        void uninit();

        static void loadImageFile(const Path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels);

        static void createImage(VkDevice              device,
                                VkPhysicalDevice      physicalDevice,
                                uint32_t              width,
                                uint32_t              height,
                                VkFormat              format,
                                VkImageTiling         tiling,
                                VkImageUsageFlags     usage,
                                VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        static void createImageView(VkDevice device, VkFormat format, VkImage image, VkImageAspectFlags imageAspect, VkImageView& outImageView);

        static void transitionImageLayout(VuDevice& vuDevice, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        static void copyBufferToImage(VuDevice& vuDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}
