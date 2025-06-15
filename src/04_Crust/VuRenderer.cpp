#include "VuRenderer.h"

#include <algorithm>                 // for fill
#include <array>                     // for array
#include <expected>                  // for expected
#include <iostream>                  // for char_traits, basic_ostream
#include <optional>                  // for optional
#include <stdexcept>                 // for runtime_error, invalid_arg...
#include <utility>                   // for move, pair
#include <vector>                    // for vector
#include <vulkan/vulkan_core.h>      // for VK_NULL_HANDLE, VK_QUEUE_F...
#include <vulkan/vulkan_enums.hpp>   // for DescriptorType, operator|
#include <vulkan/vulkan_structs.hpp> // for DescriptorSetLayoutBinding

#include "01_InnerCore/ScopeTimer.h"
#include "02_OuterCore/Color32.h"         // for Color32
#include "02_OuterCore/FixedString.h"     // for FixedString
#include "02_OuterCore/VuConfig.h"        // for MAX_FRAMES_IN_FLIGHT, MATE...
#include "03_Mantle/VuCommon.h"           // for CommandBuffer, throw_if_un...
#include "03_Mantle/VuDevice.h"           // for VuDevice
#include "03_Mantle/VuGraphicsPipeline.h" // for VuGraphicsPipeline
#include "03_Mantle/VuPhysicalDevice.h"   // for VuPhysicalDevice, VuQueueF...
#include "03_Mantle/VuSampler.h"          // for VuSampler
#include "03_Mantle/VuSwapChain.h"        // for VuSwapChain2
#include "04_Crust/VuShader.h"            // for VuShader
#include "13_Scenes/Scene_GLTF_Load.h"
#include "imgui.h"             // for GetDrawData, NewFrame, Render
#include "imgui_impl_sdl3.h"   // for ImGui_ImplSDL3_NewFrame
#include "imgui_impl_vulkan.h" // for ImGui_ImplVulkan_NewFrame
#include "SDL3/SDL_events.h"   // for SDL_PollEvent, SDL_WaitEvent
#include "SDL3/SDL_init.h"     // for SDL_Init, SDL_INIT_EVENTS
#include "SDL3/SDL_mouse.h"    // for SDL_GetMouseState
#include "SDL3/SDL_timer.h"    // for SDL_GetTicksNS, SDL_GetTicks
#include "SDL3/SDL_video.h"    // for SDL_CreateWindow, SDL_D...
#include "SDL3/SDL_vulkan.h"   // for SDL_Vulkan_CreateSurface
#include "stb_image.h"         // for stbi_image_free, stbi_uc
#include "VuMaterial.h"        // for VuMaterial, MaterialSettings
#include "VuMesh.h"            // for VuMesh

namespace Vu {
struct VuMaterial;
}

namespace Vu {

VuRenderer::VuRenderer(const VuRendererCreateInfo& createInfo) : lastCreateInfo {createInfo} {
  ScopeTimer timer;
  bool       isValidationEnabled = config::ENABLE_VALIDATION_LAYERS_LAYERS;

  // window
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);

  this->window = SDL_CreateWindow(
      "VuRenderer", Vu::config::START_WIDTH, Vu::config::START_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
  disposeStack.push([&] { SDL_DestroyWindow(this->window); });

  // instance extensions
  u32                      count                          = 0;
  const char* const*       sdlRequestedInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
  std::vector<const char*> instanceExtensions(sdlRequestedInstanceExtensions, sdlRequestedInstanceExtensions + count);

  for (auto extension : Vu::config::INSTANCE_EXTENSIONS) {
    instanceExtensions.push_back(extension);
  }
  bool enableValidationLayers = true;
#if NDEBUG
  enableValidationLayers = false;
#endif

  // instance
  auto instanceOrErr = Vu::VuInstance::make(enableValidationLayers, Vu::config::VALIDATION_LAYERS, instanceExtensions);
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
    vk::DeviceSize bufferSize = sizeof(frameConst);

    uniformBuffers[i] = VuBuffer {};
    VuBufferCreateInfo bufferCreateInfo {
        .name         = "UniformBuffer",
        .sizeInBytes  = bufferSize,
        .vkUsageFlags = vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
    };
    uniformBuffers[i] = moveOrTHROW(VuBuffer::make(*vuDevice, bufferCreateInfo));
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

  VuDeferredRenderSpace rp {vuDevice, surface};
  this->deferredRenderSpace = std::move(rp);
  deferredRenderSpace.registerImagesToBindless(*this);

  // init sync objects
  vk::SemaphoreCreateInfo semaphoreInfo {};
  vk::FenceCreateInfo     fenceInfo {};

  fenceInfo.flags = vk::FenceCreateFlagBits::eSignaled;

  uint32_t swapChainImageCount = deferredRenderSpace.vuSwapChain.images.size();
  for (size_t i = 0; i < swapChainImageCount; i++) {
    imageAvailableSemaphores.emplace_back(moveOrTHROW(vuDevice->device.createSemaphore(semaphoreInfo)));
    renderFinishedSemaphores.emplace_back(moveOrTHROW(vuDevice->device.createSemaphore(semaphoreInfo)));
    inFlightFences.emplace_back(moveOrTHROW(vuDevice->device.createFence(fenceInfo)));
  }

  initImGui();
}

bool VuRenderer::shouldWindowClose() const { return sdlEvent.type == SDL_EVENT_QUIT; }

void VuRenderer::waitForFences() const {
  auto waitRes = vuDevice->device.waitForFences(*inFlightFences[currentFrame], vk::True, UINT64_MAX);
  if (waitRes != vk::Result::eSuccess) { throw std::runtime_error("VuRenderer::waitForFences: vk::Result failed"); }
}

void VuRenderer::beginFrame() {
  waitForFences();
  std::pair<vk::Result, uint32_t> resultAndImageIndex = deferredRenderSpace.vuSwapChain.swapchain.acquireNextImage(
      UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr);

  if (resultAndImageIndex.first == vk::Result::eErrorOutOfDateKHR) {
    resetSwapChain();
    std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
  } else if (resultAndImageIndex.first != vk::Result::eSuccess &&
             resultAndImageIndex.first != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  if (resultAndImageIndex.first != vk::Result::eSuccess) {
    throw std::runtime_error("VuRenderer::beginFrame: swapchain acquire failed");
  }
  currentFrameImageIndex = resultAndImageIndex.second;

  // todo no return ?
  vuDevice->device.resetFences(*inFlightFences[currentFrame]);

  commandBuffers[currentFrame].reset();

  vk::CommandBufferBeginInfo beginInfo {};

  // todo no ret ?
  commandBuffers[currentFrame].begin(beginInfo);

  deferredRenderSpace.beginGBufferPass(commandBuffers[currentFrame], currentFrameImageIndex);

  vk::Viewport viewport {};
  viewport.x        = 0.0f;
  viewport.y        = (float)deferredRenderSpace.vuSwapChain.extend2D.height;
  viewport.width    = (float)deferredRenderSpace.vuSwapChain.extend2D.width;
  viewport.height   = -(float)deferredRenderSpace.vuSwapChain.extend2D.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  commandBuffers[currentFrame].setViewport(0, viewport);

  vk::Rect2D scissor {};
  scissor.offset = vk::Offset2D {0, 0};
  scissor.extent = deferredRenderSpace.vuSwapChain.extend2D;
  commandBuffers[currentFrame].setScissor(0, scissor);
  bindGlobalBindlessSet(commandBuffers[currentFrame]);
}

void VuRenderer::beginLightningPass() const {
  const vk::raii::CommandBuffer& cb = commandBuffers[currentFrame];
  cb.endRenderPass();

  vk::PipelineStageFlags srcStage =
      vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eLateFragmentTests;

  vk::PipelineStageFlags dstStage =
      vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eColorAttachmentOutput;

  vk::MemoryBarrier memoryBarrier;
  memoryBarrier.srcAccessMask =
      vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
  memoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

  cb.pipelineBarrier(srcStage, dstStage, {}, memoryBarrier, {}, {});

  // lightning pass
  deferredRenderSpace.beginLightningPass(commandBuffers[currentFrame], currentFrameImageIndex);
}

void VuRenderer::endFrame() {
  const vk::raii::CommandBuffer& cb = commandBuffers[currentFrame];
  cb.endRenderPass();
  cb.end();

  vk::Semaphore          waitSemaphores[]   = {imageAvailableSemaphores[currentFrame]};
  vk::PipelineStageFlags waitStages[]       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::Semaphore          signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

  vk::SubmitInfo submitInfo {};
  submitInfo.waitSemaphoreCount   = 1u;
  submitInfo.pWaitSemaphores      = waitSemaphores;
  submitInfo.pWaitDstStageMask    = waitStages;
  submitInfo.commandBufferCount   = 1u;
  submitInfo.pCommandBuffers      = &*commandBuffers[currentFrame];
  submitInfo.signalSemaphoreCount = 1u;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  vuDevice->graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

  vk::SwapchainKHR swapChains[] = {deferredRenderSpace.vuSwapChain.swapchain};

  vk::PresentInfoKHR presentInfo {};
  presentInfo.pNext              = nullptr;
  presentInfo.waitSemaphoreCount = 1u;
  presentInfo.pWaitSemaphores    = signalSemaphores;
  presentInfo.swapchainCount     = 1u;
  presentInfo.pSwapchains        = swapChains;
  presentInfo.pImageIndices      = &currentFrameImageIndex;
  presentInfo.pResults           = VK_NULL_HANDLE;

  auto result = vuDevice->graphicsQueue.presentKHR(presentInfo);

  if (result == vk::Result::eErrorOutOfDateKHR || result == vk::Result::eSuboptimalKHR) {
    resetSwapChain();
  } else if (result != vk::Result::eSuccess) {
    throw std::runtime_error("failed to present swap chain image!");
  }
  currentFrame = (currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}

void VuRenderer::bindMesh(VuMesh& mesh) {
  // we are using vertex pulling, so only index buffers we need to bind
  auto& commandBuffer = commandBuffers[currentFrame];
  auto  indexBuffer   = mesh.indexBuffer.get();
  commandBuffer.bindIndexBuffer(*indexBuffer->buffer, 0, vk::IndexType::eUint32);
}

void VuRenderer::bindMaterial(std::shared_ptr<VuMaterial>& material) {
  auto& commandBuffer = commandBuffers[currentFrame];
  bindMaterial(commandBuffer, material);
}

void VuRenderer::drawIndexed(u32 indexCount) const {
  auto& commandBuffer = commandBuffers[currentFrame];
  commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}

void VuRenderer::pushConstants(const PushConsts_RawData& pushConstant) {
  auto& commandBuffer = commandBuffers[currentFrame];
  commandBuffer.pushConstants<PushConsts_RawData>(globalPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, pushConstant);
  // vkCmdPushConstants(commandBuffer, vuDevice.globalPipelineLayout, VK_SHADER_STAGE_ALL, 0, config::PUSH_CONST_SIZE,
  //                    &pushConstant);
}

void VuRenderer::beginImgui() const {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplSDL3_ProcessEvent(&sdlEvent);
  ImGui::NewFrame();
}

void VuRenderer::endImgui() {
  auto& commandBuffer = commandBuffers[currentFrame];

  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);
}

void VuRenderer::updateFrameConstantBuffer(FrameConst_RawData ubo) const {
  auto res = uniformBuffers[currentFrame].setData(&ubo, sizeof(ubo));
  if (res != vk::Result::eSuccess) { throw std::runtime_error("failed to update frame constant buffer!"); }
}

void VuRenderer::resetSwapChain() {
  SDL_Event event;

  int width  = 0;
  int height = 0;
  SDL_GetWindowSize(window, &width, &height);
  auto minimized = (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;

  while (width <= 0 || height <= 0 || minimized) {
    SDL_GetWindowSize(window, &width, &height);
    minimized = (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
    SDL_WaitEvent(&event);
  }
  vuDevice->device.waitIdle();

  VuDeferredRenderSpace rp {vuDevice, surface};
  this->deferredRenderSpace = std::move(rp);
  deferredRenderSpace.registerImagesToBindless(*this);
}
void VuRenderer::bindGlobalBindlessSet(const vk::CommandBuffer& commandBuffer) const {

  commandBuffer.bindDescriptorSets(
      vk::PipelineBindPoint::eGraphics, globalPipelineLayout, 0, 1, &globalDescriptorSets[currentFrame], 0, nullptr);
}
void VuRenderer::preUpdate() {
  // nano => micro => mili => second
  SDL_PollEvent(&sdlEvent);
  deltaAsSecond        = (SDL_GetTicksNS() - prevTimeAsNanoSecond) / 1000.0f / 1000.0f / 1000.0f;
  prevTimeAsNanoSecond = SDL_GetTicksNS();
}
void VuRenderer::pollUserInput() {
  // SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
  float prevx = mouseX;
  float prevy = mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  mouseDeltaX = prevx - mouseX;
  mouseDeltaY = prevy - mouseY;
}
float VuRenderer::time() { return SDL_GetTicks() / 1000.0f; }
void  VuRenderer::writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const {
  vk::DescriptorBufferInfo bufferInfo {};
  bufferInfo.buffer = buffer.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range  = sizeof(FrameConst_RawData);

  vk::WriteDescriptorSet descriptorWrite {};
  descriptorWrite.dstSet          = globalDescriptorSets[setIndex];
  descriptorWrite.dstBinding      = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType  = vk::DescriptorType::eUniformBuffer;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo     = &bufferInfo;
  vuDevice->device.updateDescriptorSets(descriptorWrite, {});
}
void VuRenderer::registerToBindless(VuBuffer& vuBuffer) {
  uint32_t          bindlessIndex = bufferBindlessIndexAllocator.allocate();
  vk::DeviceAddress address       = vuBuffer.getDeviceAddress();
  // TODO handle error
  // auto view                       = std::span((uint64_t*)bdaBuffer.mapPtr, bdaBuffer.sizeInBytes / 64);
  auto res = bdaBuffer.setData(&address, sizeof(vk::DeviceAddress), bindlessIndex * sizeof(vk::DeviceAddress));
  if (res != vk::Result::eSuccess) { throw std::runtime_error("failed to set buffer data"); }
  vuBuffer.bindlessIndex = bindlessIndex;
}
void VuRenderer::registerToBindless(VuImage& vuImage) {
  uint32_t bindlessIndex = imgBindlessIndexAllocator.allocate();

  vk::DescriptorImageInfo imageInfo {};
  imageInfo.sampler     = nullptr;
  imageInfo.imageView   = vuImage.imageView;
  imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    vk::WriteDescriptorSet descriptorWrite {};
    descriptorWrite.dstSet          = globalDescriptorSets[i];
    descriptorWrite.dstBinding      = lastCreateInfo.sampledImageBinding;
    descriptorWrite.dstArrayElement = bindlessIndex;
    descriptorWrite.descriptorType  = vk::DescriptorType::eSampledImage;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo      = &imageInfo;
    vuDevice->device.updateDescriptorSets(descriptorWrite, {});
  }
  vuImage.bindlessIndex = bindlessIndex;
}
void VuRenderer::registerToBindless(VuSampler& vuSampler) {
  uint32_t                bindlessIndex = samplerBindlessIndexAllocator.allocate();
  vk::DescriptorImageInfo imageInfo {
      .sampler = vuSampler.sampler,
  };

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    vk::WriteDescriptorSet descriptorWrite {};
    descriptorWrite.dstSet          = globalDescriptorSets[i];
    descriptorWrite.dstBinding      = lastCreateInfo.samplerBinding;
    descriptorWrite.dstArrayElement = bindlessIndex;
    descriptorWrite.descriptorType  = vk::DescriptorType::eSampler;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo      = &imageInfo;
    vuDevice->device.updateDescriptorSets(descriptorWrite, {});
  }
  vuSampler.bindlessIndex = bindlessIndex;
}
void VuRenderer::uninit() { disposeStack.disposeAll(); }
void VuRenderer::initCommandPool(const VuRendererCreateInfo& info) {
  vk::CommandPoolCreateInfo poolInfo {};
  poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  poolInfo.queueFamilyIndex = vuPhysicalDevice->indices.graphicsFamily;
  auto commandPoolOrErr     = vuDevice->device.createCommandPool(poolInfo);
  // Todo
  throw_if_unexpected(commandPoolOrErr);
  commandPool = std::move(commandPoolOrErr.value());
}
void VuRenderer::initPipelineLayout() {
  std::array descSetLayouts {*globalDescriptorSetLayout};
  auto       pipelineLayoutOrErr = vuDevice->createPipelineLayout(descSetLayouts, config::PUSH_CONST_SIZE);
  throw_if_unexpected(pipelineLayoutOrErr);
  globalPipelineLayout = std::move(pipelineLayoutOrErr.value());
}
void VuRenderer::initBindlessDescriptorSetLayout(const VuRendererCreateInfo& info) {
  vk::DescriptorSetLayoutBinding ubo {};
  ubo.binding         = info.uboBinding;
  ubo.descriptorType  = vk::DescriptorType::eUniformBuffer;
  ubo.descriptorCount = info.uboCount;
  ubo.stageFlags      = vk::ShaderStageFlagBits::eAll;

  vk::DescriptorSetLayoutBinding sampler {};
  sampler.binding         = info.samplerBinding;
  sampler.descriptorType  = vk::DescriptorType::eSampler;
  sampler.descriptorCount = info.samplerCount;
  sampler.stageFlags      = vk::ShaderStageFlagBits::eAll;

  vk::DescriptorSetLayoutBinding sampledImage {};
  sampledImage.binding         = info.sampledImageBinding;
  sampledImage.descriptorType  = vk::DescriptorType::eSampledImage;
  sampledImage.descriptorCount = info.sampledImageCount;
  sampledImage.stageFlags      = vk::ShaderStageFlagBits::eAll;

  vk::DescriptorSetLayoutBinding storageImage {};
  storageImage.binding         = info.storageImageBinding;
  storageImage.descriptorType  = vk::DescriptorType::eStorageImage;
  storageImage.descriptorCount = info.storageImageCount;
  storageImage.stageFlags      = vk::ShaderStageFlagBits::eAll;

  vk::DescriptorSetLayoutBinding storageBuffer {};
  storageBuffer.binding         = info.storageBufferBinding;
  storageBuffer.descriptorType  = vk::DescriptorType::eStorageBuffer;
  storageBuffer.descriptorCount = 1;
  storageBuffer.stageFlags      = vk::ShaderStageFlagBits::eAll;

  std::array descriptorSetLayoutBindings {
      ubo,
      sampler,
      sampledImage,
      storageImage,
      storageBuffer,
  };

  vk::DescriptorSetLayoutCreateInfo descSetLayoutCreateInfo {};
  descSetLayoutCreateInfo.flags        = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPoolEXT;
  descSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
  descSetLayoutCreateInfo.pBindings    = descriptorSetLayoutBindings.data();

  const vk::DescriptorBindingFlagsEXT flag = vk::DescriptorBindingFlagBitsEXT::ePartiallyBound |
                                             vk::DescriptorBindingFlagBitsEXT::eUpdateAfterBind |
                                             vk::DescriptorBindingFlagBitsEXT::eUpdateUnusedWhilePending;

  std::array descriptorSetLayoutFlags {flag, flag, flag, flag, flag};

  vk::DescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags {};
  binding_flags.bindingCount    = descriptorSetLayoutFlags.size();
  binding_flags.pBindingFlags   = descriptorSetLayoutFlags.data();
  descSetLayoutCreateInfo.pNext = &binding_flags;

  auto descSetLayoutOrErr = vuDevice->device.createDescriptorSetLayout(descSetLayoutCreateInfo);
  throw_if_unexpected(descSetLayoutOrErr);
  globalDescriptorSetLayout = std::move(descSetLayoutOrErr.value());
}
void VuRenderer::initDefaultResources() {

  VuBufferCreateInfo stagingBufferCreateInfo {};
  stagingBufferCreateInfo.name         = "stagingBuffer";
  stagingBufferCreateInfo.sizeInBytes  = 1024 * 1024 * 64;
  stagingBufferCreateInfo.vkUsageFlags = vk::BufferUsageFlagBits::eTransferSrc;

  auto stagingOrErr = VuBuffer::make(*vuDevice, stagingBufferCreateInfo);
  throw_if_unexpected(stagingOrErr);
  this->stagingBuffer = std::move(stagingOrErr.value());
  stagingBuffer.map();

  // debugBufferHnd =
  //     createBuffer({.name           = "defaultBuffer",
  //                   .length         = 4096,
  //                   .strideInBytes  = 1,
  //                   .vkUsageFlags   = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
  //                   VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
  //                   .vmaCreateFlags = 0});

  VuBufferCreateInfo debugBufferCreateInfo {};
  debugBufferCreateInfo.name        = "debugBuffer";
  debugBufferCreateInfo.sizeInBytes = 4096;
  debugBufferCreateInfo.vkUsageFlags =
      vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;

  auto debugBufferOrErr = VuBuffer::make(*vuDevice, debugBufferCreateInfo);
  throw_if_unexpected(debugBufferOrErr);
  this->debugBuffer = std::make_shared<VuBuffer>(std::move(debugBufferOrErr.value()));

  registerToBindless(*debugBuffer);
  assert(debugBuffer->bindlessIndex == 0);

  std::vector<Color32> colorData;
  Color32              defaultColor = Color32(0.0f, 0.0f, 0.0f);
  Color32              magentaColor = Color32(1.0f, 0.0f, 1.0f);
  colorData.resize(512 * 512);

  for (int y = 0; y < 512; ++y) {
    for (int x = 0; x < 512; ++x) {
      // Determine the block (x // blockSize, y // blockSize)
      bool isMagenta         = ((x / 16) + (y / 16)) % 2 == 0;
      colorData[y * 512 + x] = isMagenta ? magentaColor : defaultColor;
    }
  }

  auto defaultImageOrErr = VuImage::make(vuDevice, {.width = 512, .height = 512, .format = vk::Format::eR8G8B8A8Srgb});
  throw_if_unexpected(defaultImageOrErr);
  defaultImage = std::make_shared<VuImage>(std::move(defaultImageOrErr.value()));
  uploadToImage(*defaultImage, reinterpret_cast<const byte*>(colorData.data()), colorData.size() * sizeof(Color32));
  registerToBindless(*defaultImage);
  assert(defaultImage->bindlessIndex == 0);

  Color32 normalColor = Color32(uint8_t(128), uint8_t(128), uint8_t(255), uint8_t(255));
  std::fill(colorData.begin(), colorData.end(), normalColor);
  auto defaultNormalImageOrErr =
      VuImage::make(vuDevice, {.width = 512, .height = 512, .format = vk::Format::eR8G8B8A8Unorm});
  throw_if_unexpected(defaultNormalImageOrErr);
  defaultNormalImage = std::make_shared<VuImage>(std::move(defaultNormalImageOrErr.value()));

  uploadToImage(
      *defaultNormalImage, reinterpret_cast<const byte*>(colorData.data()), colorData.size() * sizeof(Color32));
  registerToBindless(*defaultNormalImage);
  assert(defaultNormalImage->bindlessIndex == 1);

  // TODO pass physical props max

  auto defaultSamplerOrErr = VuSampler::make(vuDevice, {.maxAnisotropy = 16.0f});
  throw_if_unexpected(defaultSamplerOrErr);
  defaultSampler = std::make_shared<VuSampler>(std::move(defaultSamplerOrErr.value()));
  registerToBindless(*defaultSampler);
  assert(defaultSampler->bindlessIndex == 0);

  VuBufferCreateInfo matDataBufferCreateInfo {};
  matDataBufferCreateInfo.name        = "materialDataBuffer";
  matDataBufferCreateInfo.sizeInBytes = 4096;
  matDataBufferCreateInfo.vkUsageFlags =
      vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;

  auto matDatBufferOrrErr = VuBuffer::make(*vuDevice, matDataBufferCreateInfo);
  throw_if_unexpected(matDatBufferOrrErr);
  this->materialDataBuffer = std::make_shared<VuBuffer>(std::move(matDatBufferOrrErr.value()));
  registerToBindless(*materialDataBuffer);
  assert(materialDataBuffer->bindlessIndex == 1);
  materialDataBuffer->map();
}
void VuRenderer::initDescriptorPool(const VuRendererCreateInfo& info) {
  std::array<vk::DescriptorPoolSize, 5> poolSizes;

  poolSizes[0].type            = vk::DescriptorType::eUniformBuffer;
  poolSizes[0].descriptorCount = info.uboCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[1].type            = vk::DescriptorType::eSampler;
  poolSizes[1].descriptorCount = info.samplerCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[2].type            = vk::DescriptorType::eSampledImage;
  poolSizes[2].descriptorCount = info.sampledImageCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[3].type            = vk::DescriptorType::eStorageImage;
  poolSizes[3].descriptorCount = info.storageImageCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[4].type            = vk::DescriptorType::eStorageBuffer;
  poolSizes[4].descriptorCount = info.storageBufferCount * config::MAX_FRAMES_IN_FLIGHT;

  vk::DescriptorPoolCreateInfo poolInfo;
  poolInfo.flags         = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
  poolInfo.maxSets       = 2;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes    = poolSizes.data();

  auto descPoolOrErr = vuDevice->device.createDescriptorPool(poolInfo);
  throw_if_unexpected(descPoolOrErr);
  this->descriptorPool = std::move(descPoolOrErr.value());
}
void VuRenderer::initBindlessDescriptorSet() {
  std::array<vk::DescriptorSetLayout, config::MAX_FRAMES_IN_FLIGHT> globalDescLayout;
  globalDescLayout.fill(globalDescriptorSetLayout);

  vk::DescriptorSetAllocateInfo globalSetsAllocInfo;
  globalSetsAllocInfo.descriptorPool     = descriptorPool;
  globalSetsAllocInfo.descriptorSetCount = static_cast<uint32_t>(globalDescLayout.size());
  globalSetsAllocInfo.pSetLayouts        = globalDescLayout.data();

  std::array<VkDescriptorSet, config::MAX_FRAMES_IN_FLIGHT> tempDescSets {};
  vkAllocateDescriptorSets(*vuDevice->device, globalSetsAllocInfo, tempDescSets.data());
  globalDescriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
  for (uint32_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    globalDescriptorSets[i] = vk::DescriptorSet {tempDescSets[i]};
  }
}
vk::raii::CommandBuffer VuRenderer::BeginSingleTimeCommands() const {
  vk::CommandBufferAllocateInfo cbAllocInfo;
  cbAllocInfo.level              = vk::CommandBufferLevel::ePrimary;
  cbAllocInfo.commandPool        = commandPool;
  cbAllocInfo.commandBufferCount = 1;

  auto commandBufferOrErr = vuDevice->device.allocateCommandBuffers(cbAllocInfo);
  throw_if_unexpected(commandBufferOrErr);

  vk::raii::CommandBuffer commandBuffer = std::move(commandBufferOrErr.value()[0]);

  vk::CommandBufferBeginInfo beginInfo;
  beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
  // todo no return ?
  commandBuffer.begin(beginInfo);

  return commandBuffer;
}
void VuRenderer::EndSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const {
  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &*commandBuffer;

  vuDevice->graphicsQueue.submit(submitInfo);
  vuDevice->graphicsQueue.waitIdle();

  // device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}
void VuRenderer::initBindlessResourceManager(const VuRendererCreateInfo& info) {

  VuBufferCreateInfo bufferCreateInfo {
      .name        = "BDA_Buffer",
      .sizeInBytes = info.storageBufferCount * sizeof(vk::DeviceAddress),
  };
  auto bdaBufferOrErr = VuBuffer::make(*vuDevice, bufferCreateInfo);
  throw_if_unexpected(bdaBufferOrErr);
  bdaBuffer = std::move(bdaBufferOrErr.value());
  bdaBuffer.map();

  vk::DescriptorBufferInfo descBufferInfo {};
  descBufferInfo.buffer = bdaBuffer.buffer;
  descBufferInfo.range  = bdaBuffer.sizeInBytes;

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    vk::WriteDescriptorSet descriptorWrite {};
    descriptorWrite.dstSet          = globalDescriptorSets[i];
    descriptorWrite.dstBinding      = lastCreateInfo.storageBufferBinding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = vk::DescriptorType::eStorageBuffer;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo     = &descBufferInfo;

    vuDevice->device.updateDescriptorSets(descriptorWrite, {});
  }
}
// todo
VuImage VuRenderer::createImageFromAsset(const path& path, vk::Format format) {
  int   texWidth;
  int   texHeight;
  int   texChannels;
  byte* pixels;

  Vu::VuImage::loadImageFile(path, texWidth, texHeight, texChannels, reinterpret_cast<stbi_uc*&>(pixels));
  const auto imageSize = static_cast<vk::DeviceSize>(texWidth * texHeight * 4U);

  if (pixels == nullptr) { throw std::runtime_error("failed to load texture image!"); }

  u32               w = texWidth;
  u32               h = texHeight;
  VuImageCreateInfo createInfo {.width = w, .height = h, .format = format};

  auto vuImageOrrErr = VuImage::make(vuDevice, createInfo);
  throw_if_unexpected(vuImageOrrErr);
  // std::shared_ptr<VuImage> vuImage = std::make_shared<VuImage>(std::move(vuImageOrrErr.value()));
  uploadToImage(vuImageOrrErr.value(), pixels, imageSize);
  stbi_image_free(pixels);
  return std::move(vuImageOrrErr.value());
}

std::shared_ptr<VuMaterialDataHandle> VuRenderer::createMaterialDataIndex() {
  std::shared_ptr<VuMaterialDataHandle> handle        = std::make_shared<VuMaterialDataHandle>();
  VuMaterialDataHandle*                 resource      = handle.get();
  u32                                   bindlessIndex = materialDataBindlessIndexAllocator.allocate();
  resource->index                                     = bindlessIndex;
  return handle;
}
std::span<std::byte, config::MATERIAL_DATA_SIZE>
VuRenderer::getMaterialDataSpan(const std::shared_ptr<VuMaterialDataHandle>& handle) const {
  byte* dataPtr = &static_cast<byte*>(materialDataBuffer->mapPtr)[config::MATERIAL_DATA_SIZE * handle->index];
  return std::span<std::byte, config::MATERIAL_DATA_SIZE>(dataPtr, config::MATERIAL_DATA_SIZE);
}

void VuRenderer::bindMaterial(const vk::CommandBuffer& cb, const std::shared_ptr<VuMaterial>& material) {
  VuMaterial*         mat        = material.get();
  VuShader*           shader     = mat->shaderHnd.get();
  VuGraphicsPipeline& vuPipeline = shader->requestPipeline(mat->materialSettings);

  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, vuPipeline.pipeline);
  // vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
}
void VuRenderer::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
  vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands();
  vk::BufferCopy          copyRegion {};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size      = size;
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
  EndSingleTimeCommands(commandBuffer);
}
void VuRenderer::uploadToImage(const VuImage& vuImage, const byte* data, const vk::DeviceSize size) {
  // todo
  auto res = stagingBuffer.setData(data, size);

  transitionImageLayout(vuImage.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

  copyBufferToImage(stagingBuffer.buffer, vuImage.image, vuImage.lastCreateInfo.width, vuImage.lastCreateInfo.height);

  transitionImageLayout(vuImage.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
void VuRenderer::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
  vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands();

  vk::ImageMemoryBarrier barrier {};
  barrier.oldLayout                       = oldLayout;
  barrier.newLayout                       = newLayout;
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                           = image;
  barrier.subresourceRange.aspectMask     = vk::ImageAspectFlagBits::eColor;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;

  vk::PipelineStageFlags sourceStage;
  vk::PipelineStageFlags destinationStage;

  if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
    barrier.srcAccessMask = {};
    barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

    sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
    destinationStage = vk::PipelineStageFlagBits::eTransfer;
  } else if (oldLayout == vk::ImageLayout::eTransferDstOptimal &&
             newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    sourceStage      = vk::PipelineStageFlagBits::eTransfer;
    destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
  } else {
    throw std::invalid_argument("Unsupported layout transition!");
  }

  commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);

  EndSingleTimeCommands(commandBuffer);
}
void VuRenderer::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
  vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands();

  vk::BufferImageCopy region {};
  region.bufferOffset                    = 0;
  region.bufferRowLength                 = 0;
  region.bufferImageHeight               = 0;
  region.imageSubresource.aspectMask     = vk::ImageAspectFlagBits::eColor;
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = 1;
  region.imageOffset                     = vk::Offset3D {0, 0, 0};
  region.imageExtent                     = vk::Extent3D {width, height, 1};

  commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
  // vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
  EndSingleTimeCommands(commandBuffer);
}

void VuRenderer::initImGui() {

  vk::DescriptorPoolSize pool_sizes[] = {{.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1000}};

  vk::DescriptorPoolCreateInfo poolInfo {};
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes    = pool_sizes;
  poolInfo.maxSets       = 1000;
  poolInfo.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;

  auto poolOrErr   = vuDevice->device.createDescriptorPool(poolInfo);
  uiDescriptorPool = moveOrTHROW(poolOrErr);


  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  disposeStack.push([] { ImGui::DestroyContext(); });
  ImGuiIO& io = ImGui::GetIO();
  //(void) io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= 1 << 7;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForVulkan(window);
  disposeStack.push([] { ImGui_ImplSDL3_Shutdown(); });

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance                  = *vuDevice->vuPhysicalDevice->vuInstance->instance;
  init_info.PhysicalDevice            = *vuDevice->vuPhysicalDevice->physicalDevice;
  init_info.Device                    = *vuDevice->device;
  init_info.QueueFamily               = vuDevice->vuPhysicalDevice->indices.graphicsFamily;
  init_info.Queue                     = *vuDevice->graphicsQueue;
  init_info.DescriptorPool            = *uiDescriptorPool;
  init_info.MinImageCount             = 2;
  init_info.ImageCount                = 2;
  init_info.UseDynamicRendering       = false;
  init_info.RenderPass                = *deferredRenderSpace.gBufferPass->renderPass;

  ImGui_ImplVulkan_Init(&init_info);
  disposeStack.push([] { ImGui_ImplVulkan_Shutdown(); });

  ImGui_ImplVulkan_CreateFontsTexture();
  disposeStack.push([] { ImGui_ImplVulkan_DestroyFontsTexture(); });
}

} // namespace Vu
