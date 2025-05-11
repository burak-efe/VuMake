#pragma once

#include <span>                     // for span

#include "01_InnerCore/TypeDefs.h" // for u32
#include "VuCommon.h"

namespace Vu
{
struct VuQueueFamilyIndices;
}

namespace Vu::CreateUtils
{
// std::expected<vk::raii::Instance, vk::Result>
// createInstance(vk::raii::Context&     raiiContext,
//                bool                   enableValidationLayers,
//                std::span<const char*> validationLayers,
//                std::span<const char*> extensions);

// std::expected<vk::PhysicalDevice, vk::Result>
// createPhysicalDevice(const vk::Instance&    instance,
//                      const vk::SurfaceKHR&  surface,
//                      std::span<const char*> enabledExtensions);

// void createDevice(const vk::PhysicalDeviceFeatures2& features,
//              const VuQueueFamilyIndices&        indices,
//              const vk::raii::PhysicalDevice&    physicalDevice,
//              std::span<const char*>             enabledExtensions,
//              vk::raii::Device&                  outDevice,
//              vk::Queue&                         outGraphicsQueue,
//              vk::Queue&                         outPresentQueue);

// void createPipelineLayout(const vk::raii::Device&            device,
//                      std::span<vk::DescriptorSetLayout> descriptorSetLayouts,
//                      u32                                pushConstantSizeAsByte,
//                      vk::raii::PipelineLayout&          outPipelineLayout);

//void createDebugMessenger(const vk::Instance& instance, vk::DebugUtilsMessengerEXT& outDebugMessenger);
}
