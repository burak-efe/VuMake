#pragma once

#include <expected>
#include <span>                     // for span


#include "08_LangUtils/TypeDefs.h"  // for u32
#include "12_VuMakeCore/VuCommon.h"


namespace Vu::Utils
{
vk::ImageCreateInfo fillImageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent);

vk::ImageViewCreateInfo fillImageViewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags);

std::expected<u32, vk::Result> findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
                                                 u32                                     typeFilter,
                                                 vk::MemoryPropertyFlags                   requiredProperties);


void giveDebugName(vk::Device device, vk::ObjectType objType, const void* objHandle, const char* debugName);


// vk::Bool32 debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
//                        vk::DebugUtilsMessageTypeFlagsEXT             messageType,
//                        const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
//                        void*                                       pUserData);

// void fillDebugMessengerCreateInfo(vk::DebugUtilsMessengerCreateInfoEXT& createInfo);
//

bool checkDeviceExtensionSupport(vk::PhysicalDevice device, std::span<const char*> requestedExtensions);

bool isDeviceSupportExtensions(vk::PhysicalDevice       device, vk::SurfaceKHR surface,
                               std::span<const char*> enabledExtensions);

// bool checkValidationLayerSupport(std::span<const char*> validationLayers);
}
