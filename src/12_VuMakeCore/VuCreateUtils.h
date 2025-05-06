#pragma once

#include <span>                     // for span

#include <vulkan/vulkan_core.h>     // for VkInstance, VkDevice, VkPhysicalD...

#include "08_LangUtils/TypeDefs.h"  // for u32

namespace Vu { struct QueueFamilyIndices; }

namespace Vu::CreateUtils
{
    void createInstance(bool                   enableValidationLayers,
                        std::span<const char*> validationLayers,
                        std::span<const char*> extensions,
                        VkInstance&            outInstance);

    void createPhysicalDevice(const VkInstance&      instance,
                              const VkSurfaceKHR&    surface,
                              std::span<const char*> enabledExtensions,
                              VkPhysicalDevice&      outPhysicalDevice);

    void createDevice(const VkPhysicalDeviceFeatures2& features,
                      const QueueFamilyIndices&        indices,
                      const VkPhysicalDevice&          physicalDevice,
                      std::span<const char*>           enabledExtensions,
                      VkDevice&                        outDevice,
                      VkQueue&                         outGraphicsQueue,
                      VkQueue&                         outPresentQueue);

    void createPipelineLayout(VkDevice                         device,
                              std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                              u32                              pushConstantSizeAsByte,
                              VkPipelineLayout&                outPipelineLayout);

     void createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger);
}
