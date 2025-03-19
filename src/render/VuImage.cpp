#include "VuImage.h"
#include "VuBuffer.h"
#include "VuDevice.h"
#include "VuUtils.h"

void Vu::VuImage::init(const VuTextureCreateInfo& createInfo)
{
    lastCreateInfo = createInfo;
    createImage(createInfo.device,
                createInfo.physicalDevice,
                createInfo.width, createInfo.height,
                createInfo.format,
                createInfo.tiling,
                createInfo.usage,
                createInfo.properties,
                image,
                imageMemory);

    createImageView(createInfo.device, createInfo.format, image, createInfo.aspectMask, imageView);
}

void Vu::VuImage::initFromAsset(VuDevice& vuDevice, const path& path, VkFormat format)
{
    ZoneScoped;
    //Image
    int texWidth;
    int texHeight;
    int texChannels;

    stbi_uc* pixels;
    loadImageFile(path, texWidth, texHeight, texChannels, pixels);
    const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4);

    if (pixels == nullptr)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    ZoneNamedN(zone2, "staging", true);
    VuBuffer     staging{};
    VkDeviceSize size = texWidth * texHeight;
    staging.init({
                     .length = size,
                     .strideInBytes = 4,
                     .vkUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
                 });
    staging.setData(pixels, imageSize);
    stbi_image_free(pixels);

    uint32 w = texWidth;
    uint32 h = texHeight;

    init({vuDevice.device, vuDevice.physicalDevice, w, h, format});
    // createImage(vuDevice.device,
    //             vuDevice.physicalDevice,
    //             texWidth, texHeight,
    //             format,
    //             VK_IMAGE_TILING_OPTIMAL,
    //             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    //             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);

    transitionImageLayout(vuDevice, image,
                          VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    copyBufferToImage(vuDevice, staging.buffer, image, static_cast<uint32>(texWidth), static_cast<uint32>(texHeight));

    transitionImageLayout(vuDevice, image,
                          VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


    staging.uninit();
}

void Vu::VuImage::loadImageFile(const path& path, int& texWidth, int& texHeight, int& texChannels, stbi_uc*& pixels)
{
    ZoneScoped;
    pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
}

void Vu::VuImage::uninit()
{
    std::cout << "VuTexture::uninit()" << std::endl;
    vkDestroyImage(lastCreateInfo.device, image, nullptr);
    vkFreeMemory(lastCreateInfo.device, imageMemory, nullptr);
    vkDestroyImageView(lastCreateInfo.device, imageView, nullptr);
}

void Vu::VuImage::createImage(const VkDevice              device,
                              const VkPhysicalDevice      physicalDevice,
                              const uint32_t              width,
                              const uint32_t              height,
                              const VkFormat              format,
                              const VkImageTiling         tiling,
                              const VkImageUsageFlags     usage,
                              const VkMemoryPropertyFlags properties,
                              VkImage&                    image,
                              VkDeviceMemory&             imageMemory)
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
    allocInfo.memoryTypeIndex = findMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

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

void Vu::VuImage::transitionImageLayout(VuDevice& vuDevice, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer      commandBuffer = vuDevice.BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = oldLayout;
    barrier.newLayout                       = newLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = image;
    barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
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

    vuDevice.EndSingleTimeCommands(commandBuffer);
}

void Vu::VuImage::copyBufferToImage(VuDevice& vuDevice, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = vuDevice.BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset                    = 0;
    region.bufferRowLength                 = 0;
    region.bufferImageHeight               = 0;
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageOffset                     = {0, 0, 0};
    region.imageExtent                     = {
        width,
        height,
        1
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    vuDevice.EndSingleTimeCommands(commandBuffer);
}
