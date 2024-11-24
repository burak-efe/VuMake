#pragma once

#include "Common.h"

namespace std::filesystem {
    class path;
}

namespace Vu {

    struct VuTextureCreateInfo {
        std::filesystem::path path;
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB;
    };

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

        void loadImageFile(VuTextureCreateInfo createInfo, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels);

        void init(VuTextureCreateInfo createInfo);

        void uninit();

        void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    };
}
