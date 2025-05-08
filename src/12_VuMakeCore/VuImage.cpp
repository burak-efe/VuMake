#include "VuImage.h"

#include <filesystem>          // for path
#include <stdexcept>           // for runtime_error
#include <string>              // for basic_string

#include "VuCommon.h"
#include "../../external/header_onlys/stb_image.h"

#include "10_Core/Common.h"  // for vk::Check
#include "VuUtils.h"           // for findMemoryTypeIndex

void Vu::VuImage::init(vk::Device                 device, const vk::PhysicalDeviceMemoryProperties& memProps,
                       const VuImageCreateInfo& createInfo)
{
    this->device = device;

    lastCreateInfo = createInfo;
    createImage(device,
                memProps,
                createInfo.width, createInfo.height,
                createInfo.format,
                createInfo.tiling,
                createInfo.usage,
                createInfo.memProperties,
                image,
                imageMemory);

    createImageView(device, createInfo.format, image, createInfo.aspectMask, imageView);
}


void Vu::VuImage::loadImageFile(const std::filesystem::path& path, int& texWidth, int& texHeight, int& texChannels,
                                stbi_uc*&                    pixels)
{
    pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
}

void Vu::VuImage::uninit()
{
    vkDestroyImage(device, image, nullptr);
    vkFreeMemory(device, imageMemory, nullptr);
    vkDestroyImageView(device, imageView, nullptr);
}

void Vu::VuImage::createImage(const vk::Device                          device,
                              const vk::PhysicalDeviceMemoryProperties& memProps,
                              const uint32_t                          width,
                              const uint32_t                          height,
                              const vk::Format                          format,
                              const vk::ImageTiling                     tiling,
                              const vk::ImageUsageFlags                 usage,
                              const vk::MemoryPropertyFlags             properties,
                              vk::Image&                                image,
                              vk::DeviceMemory&                         imageMemory)
{
    vk::ImageCreateInfo imageInfo{};
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

    vk::MemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    vk::MemoryAllocateInfo allocInfo{};
    allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;


    allocInfo.memoryTypeIndex = Utils::findMemoryTypeIndex(memProps, memRequirements.memoryTypeBits, properties).
            value();

    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device, image, imageMemory, 0);
}

void Vu::VuImage::createImageView(vk::Device           device,
                                  vk::Format           format,
                                  vk::Image            image,
                                  vk::ImageAspectFlags imageAspect,
                                  vk::ImageView&       outImageView)
{
    vk::ImageViewCreateInfo viewInfo{};
    viewInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image                           = image;
    viewInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format                          = format;
    viewInfo.subresourceRange.aspectMask     = imageAspect;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    vk::Check(vkCreateImageView(device, &viewInfo, nullptr, &outImageView));
}
