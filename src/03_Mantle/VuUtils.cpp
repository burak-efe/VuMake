// #include "VuUtils.h"
//
// #include <iostream>           // for basic_ostream, char_traits, operator<<
// #include <set>                // for set, _Rb_tree_const_iterator
// #include <string>             // for basic_string, operator<<, operator<=>
// #include <vector>             // for vector
//
// #include "02_OuterCore/VuCtx.h"
// #include "VuTypes.h"          // for SwapChainSupportDetails, QueueFamilyInd...

// vk::ImageCreateInfo
// Vu::Utils::fillImageCreateInfo(vk::Format format, vk::ImageUsageFlags usageFlags, vk::Extent3D extent)
// {
//     vk::ImageCreateInfo info{};
//     info.imageType   = vk::ImageType::e2D;
//     info.format      = format;
//     info.extent      = extent;
//     info.mipLevels   = 1;
//     info.arrayLayers = 1;
//     info.samples     = vk::SampleCountFlagBits::e1;
//     info.tiling      = vk::ImageTiling::eOptimal;
//     info.usage       = usageFlags;
//     return info;
// }
//
// vk::ImageViewCreateInfo
// Vu::Utils::fillImageViewCreateInfo(vk::Format format, vk::Image image, vk::ImageAspectFlags aspectFlags)
// {
//     vk::ImageViewCreateInfo info{};
//     info.viewType                        = vk::ImageViewType::e2D;
//     info.image                           = image;
//     info.format                          = format;
//     info.subresourceRange.baseMipLevel   = 0;
//     info.subresourceRange.levelCount     = 1;
//     info.subresourceRange.baseArrayLayer = 0;
//     info.subresourceRange.layerCount     = 1;
//     info.subresourceRange.aspectMask     = aspectFlags;
//     return info;
// }

// std::expected<u32, vk::Result>
// Vu::Utils::findMemoryTypeIndex(const vk::PhysicalDeviceMemoryProperties& memoryProperties,
//                                u32                                       typeFilter,
//                                vk::MemoryPropertyFlags                   requiredProperties)
// {
//     for (u32 i = 0; i < memoryProperties.memoryTypeCount; ++i)
//     {
//         const bool isTypeSuitable        = (typeFilter & (1 << i)) != 0;
//         const bool hasRequiredProperties = (memoryProperties.memoryTypes[i].propertyFlags & requiredProperties) ==
//                                            requiredProperties;
//
//         if (isTypeSuitable && hasRequiredProperties)
//         {
//             return i;
//         }
//     }
//     return std::unexpected{vk::Result::eErrorOutOfDeviceMemory};
// }

// void
// Vu::Utils::giveDebugName(const vk::raii::Device& device,
//                          const vk::ObjectType    objType,
//                          const void*             objHandle,
//                          const char*             debugName)
// {
// #ifndef NDEBUG
//     //todo
//     // vk::DebugUtilsObjectNameInfoEXT info {
//     //     . objectType = objType,
//     //     . objectHandle = reinterpret_cast<uint64_t>(objHandle),
//     //     . pObjectName = debugName,
//     // };
//     // ctx::vkSetDebugUtilsObjectNameEXT(device, &info);
// #endif
//}



