#include "VuImage.h"

#include <filesystem>
#include <iostream>

#include "VuBuffer.h"
#include "VuUtils.h"

void Vu::VuImage::init(VkDevice device, const VkPhysicalDeviceMemoryProperties& memProps, const VuImageCreateInfo& createInfo)
{
    this->device         = device;
    this->physicalDevice = physicalDevice;

    lastCreateInfo = createInfo;
    createImage(device,
                memProps,
                createInfo.width, createInfo.height,
                createInfo.format,
                createInfo.tiling,
                createInfo.usage,
                createInfo.properties,
                image,
                imageMemory);

    createImageView(device, createInfo.format, image, createInfo.aspectMask, imageView);
}


void Vu::VuImage::loadImageFile(const Path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels)
{
    ZoneScoped;
    pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
}

void Vu::VuImage::uninit()
{
    vkDestroyImage(device, image, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
    vkDestroyImageView(device, imageView, nullptr);
}

void Vu::VuImage::createImage(const VkDevice                          device,
                              const VkPhysicalDeviceMemoryProperties& memProps,
                              const uint32_t                          width,
                              const uint32_t                          height,
                              const VkFormat                          format,
                              const VkImageTiling                     tiling,
                              const VkImageUsageFlags                 usage,
                              const VkMemoryPropertyFlags             properties,
                              VkImage&                                image,
                              VkDeviceMemory&                         imageMemory)
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = Utils::findMemoryTypeIndex(memProps, memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void Vu::VuImage::createImageView(VkDevice           device,
                                  VkFormat           format,
                                  VkImage            image,
                                  VkImageAspectFlags imageAspect,
                                  VkImageView&       outImageView)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = imageAspect;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    VkCheck(vkCreateImageView(device, &viewInfo, nullptr, &outImageView));
}