#include "VuRenderer.h"
//
// #include <cassert>                   // for assert
// #include <cstddef>                   // for size_t
// #include <expected>                  // for expected
// #include <functional>                // for function
// #include <memory>                    // for shared_ptr, __shared_pt...
// #include <memory_resource>           // for new_delete_resource
// #include <span>                      // for span
// #include <stdint.h>                  // for uint32_t
// #include <utility>                   // for move
// #include <vector>                    // for vector
// #include <vulkan/vulkan.hpp>         // for DeviceSize
// #include <vulkan/vulkan_core.h>      // for VkSurfaceKHR
// #include <vulkan/vulkan_enums.hpp>   // for BufferUsageFlagBits
// #include <vulkan/vulkan_handles.hpp> // for Instance, CommandPool
// #include <vulkan/vulkan_structs.hpp> // for CommandBufferAllocateInfo
//
// #include "01_InnerCore/IndexAllocator.h"     // for IndexAllocator
// #include "01_InnerCore/ScopeTimer.h"         // for ScopeTimer
// #include "01_InnerCore/TypeDefs.h"           // for u32
// #include "02_OuterCore/VuConfig.h"           // for MAX_FRAMES_IN_FLIGHT
// #include "03_Mantle/VuBuffer.h"              // for VuBuffer, VuBufferCreat...
// #include "03_Mantle/VuCommon.h"              // for CommandBuffer, Semaphore
// #include "03_Mantle/VuDeferredRenderSpace.h" // for VuDeferredRenderSpace
// #include "03_Mantle/VuDevice.h"              // for VuDevice, VuDeviceCreat...
// #include "03_Mantle/VuInstance.h"            // for VuInstance
// #include "03_Mantle/VuPhysicalDevice.h"      // for VuPhysicalDevice
// #include "03_Mantle/VuSwapChain.h"           // for VuSwapChain2
// #include "03_Mantle/VuTypes.h"               // for GPU_FrameConst, VuDispo...
// #include "SDL3/SDL_init.h"                   // for SDL_Init, SDL_INIT_EVENTS
// #include "SDL3/SDL_video.h"                  // for SDL_CreateWindow, SDL_D...
// #include "SDL3/SDL_vulkan.h"                 // for SDL_Vulkan_CreateSurface

namespace Vu {
// VuRenderer::VuRenderer(const VuRendererCreateInfo& createInfo) : lastCreateInfo {createInfo} {
//   ScopeTimer timer;
//   bool       isValidationEnabled = config::ENABLE_VALIDATION_LAYERS_LAYERS;
//
//   // window
//   assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
//
//   this->window = SDL_CreateWindow("VuRenderer", Vu::config::START_WIDTH, Vu::config::START_HEIGHT,
//                                   SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
//   disposeStack.push([&] { SDL_DestroyWindow(this->window); });
//
//   // instance extensions
//   u32                      count                          = 0;
//   const char* const*       sdlRequestedInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
//   std::vector<const char*> instanceExtensions(sdlRequestedInstanceExtensions, sdlRequestedInstanceExtensions +
//   count);
//
//   for (auto extension : Vu::config::INSTANCE_EXTENSIONS) {
//     instanceExtensions.push_back(extension);
//   }
//   bool enableValidationLayers = true;
// #if NDEBUG
//   enableValidationLayers = false;
// #endif
//
//   // instance
//   auto instanceOrErr = Vu::VuInstance::make(enableValidationLayers, Vu::config::VALIDATION_LAYERS,
//   instanceExtensions);
//   // todo
//   throw_if_unexpected(instanceOrErr);
//   this->vuInstance = std::make_shared<VuInstance>(std::move(instanceOrErr.value()));
//
//   // surface
//   VkSurfaceKHR nonRaiiSurface = nullptr;
//   bool         surfaceResult  = SDL_Vulkan_CreateSurface(window, *vuInstance->instance, nullptr, &nonRaiiSurface);
//   assert(surfaceResult == true);
//
//   // instance
//   this->surface = std::make_shared<vk::raii::SurfaceKHR>(vuInstance->instance, nonRaiiSurface);
//
//   // phyDevice
//   auto phyDevOrErr = Vu::VuPhysicalDevice::make(vuInstance, *surface, Vu::config::DEVICE_EXTENSIONS);
//   throw_if_unexpected(phyDevOrErr);
//   this->vuPhysicalDevice = std::make_shared<VuPhysicalDevice>(std::move(phyDevOrErr.value()));
//
//   // device
//   VuDeviceCreateFeatureChain defaultFeatureChain {};
//
//   auto vuDeviceOrErr = VuDevice::make(vuPhysicalDevice, defaultFeatureChain.deviceFeatures2,
//   config::DEVICE_EXTENSIONS); throw_if_unexpected(vuDeviceOrErr); this->vuDevice =
//   std::make_shared<VuDevice>(std::move(vuDeviceOrErr.value()));
//
//   imgBindlessIndexAllocator          = IndexAllocator {createInfo.sampledImageCount,
//   std::pmr::new_delete_resource()}; samplerBindlessIndexAllocator      = IndexAllocator {createInfo.samplerCount,
//   std::pmr::new_delete_resource()}; bufferBindlessIndexAllocator       = IndexAllocator
//   {createInfo.storageBufferCount, std::pmr::new_delete_resource()}; materialDataBindlessIndexAllocator =
//   IndexAllocator {1024, std::pmr::new_delete_resource()};
//
//   initCommandPool(createInfo);
//   initBindlessDescriptorSetLayout(createInfo);
//   initDescriptorPool(createInfo);
//   initPipelineLayout();
//   initBindlessDescriptorSet();
//   initBindlessResourceManager(createInfo);
//   initDefaultResources();
//
//
//   // init uniform buffers
//
//   uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
//   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     vk::DeviceSize bufferSize = sizeof(GPU_FrameConst);
//
//     uniformBuffers[i] = VuBuffer {};
//     VuBufferCreateInfo bufferCreateInfo {
//         .name         = "UniformBuffer",
//         .sizeInBytes  = bufferSize,
//         .vkUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
//     };
//     uniformBuffers[i] = move_or_throw(VuBuffer::make(*vuDevice, bufferCreateInfo));
//     uniformBuffers[i].map();
//   }
//   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     writeUBO_ToGlobalPool(uniformBuffers[i], 0, i);
//   }
//
//   // init command buffers
//
//   vk::CommandBufferAllocateInfo allocInfo {};
//   allocInfo.commandPool        = commandPool;
//   allocInfo.level              = vk::CommandBufferLevel::ePrimary;
//   allocInfo.commandBufferCount = static_cast<u32>(config::MAX_FRAMES_IN_FLIGHT);
//
//   auto commandBuffersOrErr = vuDevice->device.allocateCommandBuffers(allocInfo);
//
//   throw_if_unexpected(commandBuffersOrErr);
//   commandBuffers.clear();
//   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     commandBuffers.emplace_back(std::move(commandBuffersOrErr.value()[i]));
//   }
//
//   VuDeferredRenderSpace rp {vuDevice, surface};
//   this->deferredRenderSpace = std::move(rp);
//   deferredRenderSpace.registerImagesToBindless(*this);
//
//
//   // init sync objects
//   vk::SemaphoreCreateInfo semaphoreInfo {};
//   vk::FenceCreateInfo     fenceInfo {};
//
//   fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;
//
//   uint32_t swapChainImageCount = deferredRenderSpace.vuSwapChain.images.size();
//   for (size_t i = 0; i < swapChainImageCount; i++) {
//     imageAvailableSemaphores.emplace_back(move_or_throw(vuDevice->device.createSemaphore(semaphoreInfo)));
//     renderFinishedSemaphores.emplace_back(move_or_throw(vuDevice->device.createSemaphore(semaphoreInfo)));
//     inFlightFences.emplace_back(move_or_throw(vuDevice->device.createFence(fenceInfo)));
//   }
//
// }

// initImGui();
//}
//
//
//
// void
// VuRenderer::bindGlobalBindlessSet(const vk::CommandBuffer& commandBuffer) {
//   vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuDevice.globalPipelineLayout, 0, 1,
//                           &vuDevice.globalDescriptorSets[currentFrame], 0, nullptr);
// }
//
// void
// VuRenderer::uninit() {
//   // std::cout << "VuRenderer::uninit" << std::endl;
//   vkDeviceWaitIdle(vuDevice.device);
//   disposeStack.disposeAll();
// }
} // namespace Vu
