#pragma once


#include "Common.h"

namespace std::filesystem {
    class path;
}

class VuTexture {
public:
    VkImage TextureImage{};
    VkDeviceMemory TextureImageMemory{};

    explicit VuTexture(std::filesystem::path path);

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);


    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    void Dispose();
};


