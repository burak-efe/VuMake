#include "VuCreateUtils.h"

#include <cstdint>   // for uint32_t
#include <optional>  // for optional
#include <set>       // for set, _Rb_tree_const_iterator
#include <stdexcept> // for runtime_error
#include <string>    // for basic_string, string
#include <vector>    // for vector

#include "02_OuterCore/Common.h" // for vk::Check
#include "02_OuterCore/VuCtx.h"
#include "VuCommon.h"
#include "VuTypes.h" // for QueueFamilyIndices
#include "VuUtils.h" // for fillDebugMessengerCreateInfo,

// std::expected<vk::PhysicalDevice, vk::Result>
// Vu::CreateUtils::createPhysicalDevice(const vk::Instance&    instance,
//                                       const vk::SurfaceKHR&  surface,
//                                       std::span<const char*> enabledExtensions)
// {
//
//     uint32_t deviceCount = 0;
//
//     VK_RET_ON_FAIL(instance.enumeratePhysicalDevices(&deviceCount, nullptr));
//
//     if (deviceCount == 0)
//     {
//         return std::unexpected{vk::Result::eErrorInitializationFailed};
//     }
//
//     std::vector<vk::PhysicalDevice> devices(deviceCount);
//     VK_RET_ON_FAIL(instance.enumeratePhysicalDevices( &deviceCount, devices.data()));
//
//     for (const auto& device : devices)
//     {
//         if (Utils::isDeviceSupportExtensions(device, surface, enabledExtensions))
//         {
//             return device;
//         }
//     }
//
//     return std::unexpected{vk::Result::eErrorInitializationFailed};
// }

// void
// Vu::CreateUtils::createDevice(const vk::PhysicalDeviceFeatures2& features,
//                               const VuQueueFamilyIndices&        indices,
//                               const vk::raii::PhysicalDevice&    physicalDevice,
//                               std::span<const char*>             enabledExtensions,
//                               vk::raii::Device&                  outDevice,
//                               vk::Queue&                         outGraphicsQueue,
//                               vk::Queue&                         outPresentQueue) {
//
//   std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos {};
//   std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
//
//   float queuePriority = 1.0f;
//   for (uint32_t queueFamily : uniqueQueueFamilies) {
//     vk::DeviceQueueCreateInfo queueCreateInfo {};
//     queueCreateInfo.queueFamilyIndex = queueFamily;
//     queueCreateInfo.queueCount       = 1;
//     queueCreateInfo.pQueuePriorities = &queuePriority;
//     queueCreateInfos.push_back(queueCreateInfo);
//   }
//
//   vk::DeviceCreateInfo createInfo {};
//   createInfo.pNext                   = &features;
//   createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
//   createInfo.pQueueCreateInfos       = queueCreateInfos.data();
//   createInfo.pEnabledFeatures        = nullptr;
//   createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
//   createInfo.ppEnabledExtensionNames = enabledExtensions.data();
//   // create
//   {
//     auto deviceOrErr = physicalDevice.createDevice(createInfo);
//     // todo
//     throw_if_unexpected(deviceOrErr);
//     outDevice = std::move(deviceOrErr.value());
//   }
//
//   // queue
//   {
//     auto graphicsQueueOrErr = outDevice.getQueue(indices.graphicsFamily.value(), 0);
//     auto presentQueueOrErr  = outDevice.getQueue(indices.presentFamily.value(), 0);
//
//     // todo
//     throw_if_unexpected(graphicsQueueOrErr);
//     throw_if_unexpected(presentQueueOrErr);
//
//     outGraphicsQueue = graphicsQueueOrErr.value();
//     outPresentQueue  = presentQueueOrErr.value();
//   }
// }

// void
// Vu::CreateUtils::createPipelineLayout(const vk::raii::Device&                  device,
//                                       const std::span<vk::DescriptorSetLayout> descriptorSetLayouts,
//                                       const u32                                pushConstantSizeAsByte,
//                                       vk::raii::PipelineLayout&                outPipelineLayout) {
//
//   vk::PushConstantRange pcRange {};
//   pcRange.stageFlags = vk::ShaderStageFlagBits::eAll;
//   pcRange.size       = pushConstantSizeAsByte;
//
//   vk::PipelineLayoutCreateInfo pipelineLayoutInfo {};
//   pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
//   pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
//   pipelineLayoutInfo.pushConstantRangeCount = 1;
//   pipelineLayoutInfo.pPushConstantRanges    = &pcRange;
//
//   // vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout);
//   // todo
//   auto pipelineLayoutOrErr = device.createPipelineLayout(pipelineLayoutInfo);
//   outPipelineLayout        = std::move(pipelineLayoutOrErr.value());
// }

// void
// Vu::CreateUtils::createDebugMessenger(const vk::Instance& instance, vk::DebugUtilsMessengerEXT& outDebugMessenger) {
//   vk::DebugUtilsMessengerCreateInfoEXT createInfo {};
//   Utils::fillDebugMessengerCreateInfo(createInfo);
//   vk::Check(Vu::ctx::vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &outDebugMessenger));
// }
