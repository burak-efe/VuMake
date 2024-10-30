#pragma once

#include "Common.h"
//#include "VuCtx.h"
//#include "VuUtils.h"

namespace std::filesystem {
    class path;
}

namespace Vu {
    struct VuTexture {
    private:
        void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

        void CreateImageView();

    public:
        inline static std::vector<VuTexture> allTextures;

        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;

        void Alloc(std::filesystem::path path);

        void Dispose();

        void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    };
}
