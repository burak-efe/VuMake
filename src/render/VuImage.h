#pragma once

#include "Common.h"


namespace Vu
{
    struct VuDevice;

    struct VuTextureCreateInfo
    {
        //VuDevice*        vuDevice;
        //path             path;
        VkDevice         device;
        VkPhysicalDevice physicalDevice;
        uint32_t         width;
        uint32_t         height;
        VkFormat         format = VK_FORMAT_R8G8B8A8_SRGB;
    };


    struct VuImage
    {
        VuTextureCreateInfo lastCreateInfo;
        VkImage             image;
        VkDeviceMemory      imageMemory;
        VkImageView         imageView;

        void init(const VuTextureCreateInfo& createInfo);

        void initFromAsset(VuDevice& vuDevice, const path& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);

        void uninit();

        static void loadImageFile(const path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels);

        static void createImage(VkDevice              device,
                                VkPhysicalDevice      physicalDevice,
                                uint32_t              width,
                                uint32_t              height,
                                VkFormat              format,
                                VkImageTiling         tiling,
                                VkImageUsageFlags     usage,
                                VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        static void createImageView(VkDevice device, VkFormat format, VkImage image, VkImageView& outImageView);

        static void transitionImageLayout(VuDevice& vuDevice, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        static void copyBufferToImage(VuDevice& vuDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    };
}
