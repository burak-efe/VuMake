#pragma once

#include "Common.h"
#include "VuCtx.h"
#include "VuUtils.h"
#include "VuBuffer.h"

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
        void createImage(uint32_t width,
                         uint32_t height,
                         VkFormat format,
                         VkImageTiling tiling,
                         VkImageUsageFlags usage,
                         VkMemoryPropertyFlags properties,
                         VkImage& image,
                         VkDeviceMemory& imageMemory) {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = width;
            imageInfo.extent.height = height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = format;
            imageInfo.tiling = tiling;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = usage;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            if (vkCreateImage(ctx::device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
                throw std::runtime_error("failed to create image!");
            }

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(ctx::device, image, &memRequirements);

            VkMemoryAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            allocInfo.allocationSize = memRequirements.size;
            allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

            if (vkAllocateMemory(ctx::device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
                throw std::runtime_error("failed to allocate image memory!");
            }

            vkBindImageMemory(ctx::device, image, imageMemory, 0);
        }


        void createImageView(VkFormat format) {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = textureImage;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = format;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            VkCheck(vkCreateImageView(ctx::device, &viewInfo, nullptr, &textureImageView));
        }

        void loadImageFile(VuTextureCreateInfo createInfo, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels) {
            ZoneScoped;
            pixels = stbi_load(createInfo.path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        }
    public:
        VkImage textureImage;
        VkDeviceMemory textureImageMemory;
        VkImageView textureImageView;

        void init(VuTextureCreateInfo createInfo) {
            ZoneScoped;
            std::cout << "VuTexture::init()" << std::endl;
            //Image
            int texWidth;
            int texHeight;
            int texChannels;

            stbi_uc* pixels;
            loadImageFile(createInfo, texWidth, texHeight, texChannels, pixels);
            const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

            if (pixels == nullptr) {
                throw std::runtime_error("failed to load texture image!");
            }

            ZoneNamedN(zone2, "staging", true);
            VuBuffer staging{};
            VkDeviceSize size = texWidth * texHeight;
            staging.init({
                .lenght = size,
                .strideInBytes = 4,
                .vkUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
            });
            staging.SetData(pixels, imageSize);

            stbi_image_free(pixels);


            createImage(texWidth, texHeight, createInfo.format, VK_IMAGE_TILING_OPTIMAL,
                        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        textureImage,
                        textureImageMemory);

            transitionImageLayout(textureImage,
                                  VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

            copyBufferToImage(staging.buffer, textureImage,
                              static_cast<uint32>(texWidth), static_cast<uint32>(texHeight));

            transitionImageLayout(textureImage,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

            createImageView(createInfo.format);

            staging.uninit();


        }

        void uninit() {
            std::cout << "VuTexture::uninit()" << std::endl;
            vkDestroyImage(ctx::device, textureImage, nullptr);
            vkFreeMemory(ctx::device, textureImageMemory, nullptr);
            vkDestroyImageView(ctx::device, textureImageView, nullptr);
        }

        void transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {

            VkCommandBuffer commandBuffer = ctx::BeginSingleTimeCommands();
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = oldLayout;
            barrier.newLayout = newLayout;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = image;
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            VkPipelineStageFlags sourceStage;
            VkPipelineStageFlags destinationStage;

            if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
                barrier.srcAccessMask = 0;
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
            } else {
                throw std::invalid_argument("unsupported layout transition!");
            }

            vkCmdPipelineBarrier(
                commandBuffer,
                sourceStage, destinationStage,
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier
            );

            ctx::EndSingleTimeCommands(commandBuffer);
        }

        void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
            VkCommandBuffer commandBuffer = ctx::BeginSingleTimeCommands();

            VkBufferImageCopy region{};
            region.bufferOffset = 0;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = 0;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = {0, 0, 0};
            region.imageExtent = {
                width,
                height,
                1
            };

            vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

            ctx::EndSingleTimeCommands(commandBuffer);
        }

    };

}
