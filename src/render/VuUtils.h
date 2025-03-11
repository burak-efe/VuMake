#pragma once

#include <fstream>
#include "Common.h"

namespace Vu {


    inline std::vector<char> readFile(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t            fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }


    inline VkImageCreateInfo fillImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
        VkImageCreateInfo info = {};
        info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext             = nullptr;

        info.imageType = VK_IMAGE_TYPE_2D;

        info.format = format;
        info.extent = extent;

        info.mipLevels   = 1;
        info.arrayLayers = 1;
        info.samples     = VK_SAMPLE_COUNT_1_BIT;
        info.tiling      = VK_IMAGE_TILING_OPTIMAL;
        info.usage       = usageFlags;

        return info;
    }

    inline VkImageViewCreateInfo fillImageViewCreateInfo(VkFormat format, VkImage image, VkImageAspectFlags aspectFlags) {
        //build a image-view for the depth image to use for rendering
        VkImageViewCreateInfo info = {};
        info.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        info.pNext                 = nullptr;

        info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        info.image                           = image;
        info.format                          = format;
        info.subresourceRange.baseMipLevel   = 0;
        info.subresourceRange.levelCount     = 1;
        info.subresourceRange.baseArrayLayer = 0;
        info.subresourceRange.layerCount     = 1;
        info.subresourceRange.aspectMask     = aspectFlags;

        return info;
    }


    inline uint32 findMemoryType(VkPhysicalDevice physicalDevice, uint32 typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties{};
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
        for (uint32 i = 0; i < memProperties.memoryTypeCount; i++) {
            if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    static void createPipelineLayout(const VkDevice                         device,
                                     const std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                                     const uint32                           pushConstantSizeAsByte,
                                     VkPipelineLayout&                      outPipelineLayout) {
        ZoneScoped;

        //push constants
        VkPushConstantRange pcRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0,
            .size = pushConstantSizeAsByte,
        };

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
        pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges    = &pcRange;

        VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout));
    }


    inline void giveDebugName(const VkDevice device, const VkObjectType objType, const void* objHandle, const char* debugName) {
#ifndef NDEBUG
        VkDebugUtilsObjectNameInfoEXT info{
            . sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
            . pNext = nullptr,
            . objectType = objType,
            . objectHandle = reinterpret_cast<uint64_t>(objHandle),
            . pObjectName = debugName,
        };
        vkSetDebugUtilsObjectNameEXT(device, &info);
#endif
    }

}
