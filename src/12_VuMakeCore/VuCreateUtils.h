#pragma once

#include <span>

#include "10_Core/VuCommon.h"

#include "VuTypes.h"

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

    void createPipelineLayout(const VkDevice                         device,
                              const std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                              const uint32                           pushConstantSizeAsByte,
                              VkPipelineLayout&                      outPipelineLayout);

    VkResult createDebugUtilsMessengerEXT(VkInstance                                instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks*              pAllocator,
                                          VkDebugUtilsMessengerEXT*                 pDebugMessenger);

    void destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                       VkDebugUtilsMessengerEXT     debugMessenger,
                                       const VkAllocationCallbacks* pAllocator);

    void createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger);
}
