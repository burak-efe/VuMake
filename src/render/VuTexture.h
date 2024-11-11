#pragma once

#include "Common.h"

namespace std::filesystem {
    class path;
}

namespace Vu {

    // struct VuTextureCreateInfo {
    //     uint32 width;
    //     uint32 height = ;
    //     VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    // };
    struct VuTexture {
    private:
        void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        void createImageView(VkFormat format);

    public:
       // inline static std::vector<VuTexture> allTextures;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;

        void alloc(std::filesystem::path path, VkFormat format);

        void Dispose();

        void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    };
}
