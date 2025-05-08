#pragma once

#include <span>                     // for span

#include "08_LangUtils/TypeDefs.h"  // for u32
#include "VuCommon.h"

namespace Vu
{
struct QueueFamilyIndices;
}

namespace Vu::CreateUtils
{
std::expected<vk::raii::Instance, vk::Result>
createInstance(vk::raii::Context&     raiiContext,
               bool                   enableValidationLayers,
               std::span<const char*> validationLayers,
               std::span<const char*> extensions);

std::expected<vk::PhysicalDevice, vk::Result>
createPhysicalDevice(const vk::Instance&    instance,
                     const vk::SurfaceKHR&  surface,
                     std::span<const char*> enabledExtensions);

void createDevice(const vk::PhysicalDeviceFeatures2& features,
                  const QueueFamilyIndices&          indices,
                  const vk::PhysicalDevice&          physicalDevice,
                  std::span<const char*>             enabledExtensions,
                  vk::Device&                        outDevice,
                  vk::Queue&                         outGraphicsQueue,
                  vk::Queue&                         outPresentQueue);

void createPipelineLayout(vk::Device                         device,
                          std::span<vk::DescriptorSetLayout> descriptorSetLayouts,
                          u32                                pushConstantSizeAsByte,
                          vk::PipelineLayout&                outPipelineLayout);

void createDebugMessenger(const vk::Instance& instance, vk::DebugUtilsMessengerEXT& outDebugMessenger);
}
