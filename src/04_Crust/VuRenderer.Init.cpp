#include "VuRenderer.h"

#include <cassert>    // for assert
#include <cstddef>    // for size_t
#include <format>     // for format
#include <functional> // for function
#include <iostream>   // for basic_ostream, operator<<
#include <span>       // for span
#include <string>     // for char_traits, basic_string
#include <vector>     // for vector

#include "01_InnerCore/ScopeTimer.h" // for ScopeTimer
#include "01_InnerCore/TypeDefs.h"   // for u32
#include "02_OuterCore/VuConfig.h"
#include "03_Mantle/VuDeferredRenderSpace.h"
#include "imgui.h"             // for CreateContext, DestroyContext
#include "imgui_impl_sdl3.h"   // for ImGui_ImplSDL3_InitForVulkan
#include "imgui_impl_vulkan.h" // for ImGui_ImplVulkan_InitInfo
#include "SDL3/SDL_init.h"     // for SDL_Init, SDL_Quit, SDL_INI...
#include "SDL3/SDL_video.h"    // for SDL_CreateWindow, SDL_Destr...
#include "SDL3/SDL_vulkan.h"   // for SDL_Vulkan_CreateSurface

namespace Vu {
VuRenderer::VuRenderer(const VuRendererCreateInfo& createInfo) : lastCreateInfo {createInfo} {
  ScopeTimer timer;
  bool       isValidationEnabled = config::ENABLE_VALIDATION_LAYERS_LAYERS;

  // window
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);

  this->window = SDL_CreateWindow("VuRenderer", Vu::config::START_WIDTH, Vu::config::START_HEIGHT,
                                  SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  disposeStack.push([&] { SDL_DestroyWindow(this->window); });

  // instance extensions
  u32                      count                          = 0;
  const char* const*       sdlRequestedInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
  std::vector<const char*> instanceExtensions(sdlRequestedInstanceExtensions, sdlRequestedInstanceExtensions + count);

  for (auto extension : Vu::config::INSTANCE_EXTENSIONS) {
    instanceExtensions.push_back(extension);
  }

  // instance
  auto instanceOrErr = Vu::VuInstance::make(true, Vu::config::VALIDATION_LAYERS, instanceExtensions);
  // todo
  throw_if_unexpected(instanceOrErr);
  this->vuInstance = std::make_shared<VuInstance>(std::move(instanceOrErr.value()));

  // surface
  VkSurfaceKHR nonRaiiSurface = nullptr;
  bool         surfaceResult  = SDL_Vulkan_CreateSurface(window, *vuInstance->instance, nullptr, &nonRaiiSurface);
  assert(surfaceResult == true);

  // instance
  this->surface = std::make_shared<vk::raii::SurfaceKHR>(vuInstance->instance, nonRaiiSurface);

  // phyDevice
  auto phyDevOrErr = Vu::VuPhysicalDevice::make(vuInstance, *surface, Vu::config::DEVICE_EXTENSIONS);
  throw_if_unexpected(phyDevOrErr);
  this->vuPhysicalDevice = std::make_shared<VuPhysicalDevice>(std::move(phyDevOrErr.value()));

  // device
  VuDeviceCreateFeatureChain defaultFeatureChain {};

  auto vuDeviceOrErr = VuDevice::make(vuPhysicalDevice, defaultFeatureChain.deviceFeatures2, config::DEVICE_EXTENSIONS);
  throw_if_unexpected(vuDeviceOrErr);
  this->vuDevice = std::make_shared<VuDevice>(std::move(vuDeviceOrErr.value()));

  VuDeferredRenderSpace rp {vuDevice, surface};
  this->deferredRenderSpace = std::move(rp);

  imgBindlessIndexAllocator          = IndexAllocator {createInfo.sampledImageCount, std::pmr::new_delete_resource()};
  samplerBindlessIndexAllocator      = IndexAllocator {createInfo.samplerCount, std::pmr::new_delete_resource()};
  bufferBindlessIndexAllocator       = IndexAllocator {createInfo.storageBufferCount, std::pmr::new_delete_resource()};
  materialDataBindlessIndexAllocator = IndexAllocator {1024, std::pmr::new_delete_resource()};
  initCommandPool(createInfo);
  initBindlessDescriptorSetLayout(createInfo);
  initDescriptorPool(createInfo);
  initPipelineLayout();
  initBindlessDescriptorSet();
  initBindlessResourceManager(createInfo);
  initDefaultResources();

  // init uniform buffers

  uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    vk::DeviceSize bufferSize = sizeof(GPU_FrameConst);

    uniformBuffers[i] = VuBuffer {};
    VuBufferCreateInfo bufferCreateInfo {
        .name         = "UniformBuffer",
        .sizeInBytes  = bufferSize,
        .vkUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
    };
    uniformBuffers[i] = move_or_throw(VuBuffer::make(*vuDevice, bufferCreateInfo));
    uniformBuffers[i].map();
  }
  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    writeUBO_ToGlobalPool(uniformBuffers[i], 0, i);
  }

  // init command buffers

  vk::CommandBufferAllocateInfo allocInfo {};
  allocInfo.commandPool        = commandPool;
  allocInfo.level              = vk::CommandBufferLevel::ePrimary;
  allocInfo.commandBufferCount = static_cast<u32>(config::MAX_FRAMES_IN_FLIGHT);

  auto commandBuffersOrErr = vuDevice->device.allocateCommandBuffers(allocInfo);

  throw_if_unexpected(commandBuffersOrErr);
  commandBuffers.clear();
  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    commandBuffers.emplace_back(std::move(commandBuffersOrErr.value()[i]));
  }

  // init sync objects

  vk::SemaphoreCreateInfo semaphoreInfo {};
  vk::FenceCreateInfo     fenceInfo {};

  fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

  uint32_t swapChainImageCount = deferredRenderSpace.vuSwapChain.images.size();
  for (size_t i = 0; i < swapChainImageCount; i++) {
    imageAvailableSemaphores.emplace_back(move_or_throw(vuDevice->device.createSemaphore(semaphoreInfo)));
    renderFinishedSemaphores.emplace_back(move_or_throw(vuDevice->device.createSemaphore(semaphoreInfo)));
    inFlightFences.emplace_back(move_or_throw(vuDevice->device.createFence(fenceInfo)));
  }
}

//
//   vuDevice = VuDevice {{
//       .instance                = instance,
//       .physicalDevice          = physicalDevice,
//       .enableValidationLayers  = config::ENABLE_VALIDATION_LAYERS_LAYERS,
//       .physicalDeviceFeatures2 = deviceFeatures2,
//       .surface                 = surface,
//       .deviceExtensions        = config::DEVICE_EXTENSIONS,
//
//       .uboBinding           = config::BINDLESS_UNIFORM_BUFFER_BINDING,
//       .samplerBinding       = config::BINDLESS_SAMPLER_BINDING,
//       .sampledImageBinding  = config::BINDLESS_SAMPLED_IMAGE_BINDING,
//       .storageImageBinding  = config::BINDLESS_STORAGE_IMAGE_BINDING,
//       .storageBufferBinding = config::BINDLESS_STORAGE_BUFFER_BINDING,
//
//       .uboCount           = config::BINDLESS_UNIFORM_BUFFER_COUNT,
//       .samplerCount       = config::BINDLESS_SAMPLER_COUNT,
//       .sampledImageCount  = config::BINDLESS_SAMPLED_IMAGE_COUNT,
//       .storageImageCount  = config::BINDLESS_STORAGE_IMAGE_COUNT,
//       .storageBufferCount = config::BINDLESS_STORAGE_BUFFER_COUNT,
//   }};
//   disposeStack.push([&] { vuDevice.uninit(); });
// }

// init swapchain
// {
//   swapChain = {&vuDevice, surface};
//   disposeStack.push([&] { swapChain.uninit(); });
// }

//
// // init uniform buffers
// {
//   uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
//   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     vk::DeviceSize bufferSize = sizeof(GPU_FrameConst);
//
//     uniformBuffers[i] = VuBuffer {};
//     uniformBuffers[i].init(
//         vuDevice.device, vuDevice.vma,
//         {.name           = "UniformBuffer",
//          .length         = 1,
//          .strideInBytes  = bufferSize,
//          .vkUsageFlags   = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
//          .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
//          .vmaCreateFlags =
//              VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT});
//   }
//   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     vuDevice.writeUBO_ToGlobalPool(uniformBuffers[i], 0, i);
//   }
//
//   disposeStack.push([this] {
//     for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//       auto v = uniformBuffers;
//       v[i].uninit();
//     }
//   });
// }
// // init command buffers
// {
//   commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
//
//   vk::CommandBufferAllocateInfo allocInfo {};
//   allocInfo.commandPool        = vuDevice.commandPool;
//   allocInfo.level              = vk::CommandBufferLevel::ePrimary;
//   allocInfo.commandBufferCount = static_cast<u32>(commandBuffers.size());
//
//   VkCheck(vkAllocateCommandBuffers(vuDevice.device, &allocInfo, commandBuffers.data()));
//   for (u32 i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//     std::string name = std::format("Command Buffer {}", i);
//     Utils::giveDebugName(vuDevice.device, VK_OBJECT_TYPE_COMMAND_BUFFER, commandBuffers[i], name.c_str());
//   }
// }
// // init sync objects
// {
//   vk::SemaphoreCreateInfo semaphoreInfo {};
//   semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//   vk::FenceCreateInfo fenceInfo {};
//   fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//   fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//   uint32_t swapChainImageCount = swapChain.swapChainImages.size();
//   imageAvailableSemaphores.resize(swapChainImageCount);
//   renderFinishedSemaphores.resize(swapChainImageCount);
//   inFlightFences.resize(swapChainImageCount);
//   for (size_t i = 0; i < swapChainImageCount; i++) {
//     vk::Check(vkCreateSemaphore(vuDevice.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
//     vk::Check(vkCreateSemaphore(vuDevice.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
//     vk::Check(vkCreateFence(vuDevice.device, &fenceInfo, nullptr, &inFlightFences[i]));
//   }
//
//   disposeStack.push([this, swapChainImageCount] {
//     for (size_t i = 0; i < swapChainImageCount; i++) {
//       vkDestroySemaphore(vuDevice.device, imageAvailableSemaphores[i], nullptr);
//       vkDestroySemaphore(vuDevice.device, renderFinishedSemaphores[i], nullptr);
//       vkDestroyFence(vuDevice.device, inFlightFences[i], nullptr);
//     }
//   });
// }

// initImGui();
//}
//
// void
// VuRenderer::initImGui() {
//   vk::DescriptorPoolSize pool_sizes[] = {
//       {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
//   };
//
//   vk::DescriptorPoolCreateInfo poolInfo {};
//   poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//   poolInfo.poolSizeCount = 1;
//   poolInfo.pPoolSizes    = pool_sizes;
//   poolInfo.maxSets       = 1000;
//   poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//
//   vk::Check(vkCreateDescriptorPool(vuDevice.device, &poolInfo, nullptr, &vuDevice.uiDescriptorPool));
//   disposeStack.push([&] { vkDestroyDescriptorPool(vuDevice.device, vuDevice.uiDescriptorPool, nullptr); });
//
//   // Setup Dear ImGui context
//   IMGUI_CHECKVERSION();
//   ImGui::CreateContext();
//   disposeStack.push([] { ImGui::DestroyContext(); });
//   ImGuiIO& io = ImGui::GetIO();
//   //(void) io;
//   io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
//   // io.ConfigFlags |= 1 << 7;
//   // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//   // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//   // Setup Dear ImGui style
//   ImGui::StyleColorsDark();
//
//   ImGui_ImplSDL3_InitForVulkan(ctx::window);
//   disposeStack.push([] { ImGui_ImplSDL3_Shutdown(); });
//
//   ImGui_ImplVulkan_InitInfo init_info = {};
//   init_info.Instance                  = vuDevice.instance;
//   init_info.PhysicalDevice            = vuDevice.physicalDevice;
//   init_info.Device                    = vuDevice.device;
//   init_info.QueueFamily    = VuSwapChain::findQueueFamilies(vuDevice.physicalDevice,
//   surface).graphicsFamily.value(); init_info.Queue          = vuDevice.graphicsQueue; init_info.DescriptorPool =
//   vuDevice.uiDescriptorPool; init_info.MinImageCount  = 2; init_info.ImageCount     = 2;
//   init_info.UseDynamicRendering = false;
//   init_info.RenderPass          = swapChain.gBufferPass.renderPass;
//
//   ImGui_ImplVulkan_Init(&init_info);
//   disposeStack.push([] { ImGui_ImplVulkan_Shutdown(); });
//
//   ImGui_ImplVulkan_CreateFontsTexture();
//   disposeStack.push([] { ImGui_ImplVulkan_DestroyFontsTexture(); });
// }
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
