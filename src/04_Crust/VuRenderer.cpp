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
#include <vulkan/vulkan_funcs.hpp>   // for CommandBuffer::bindDescrip...
#include <vulkan/vulkan_structs.hpp> // for DescriptorSetLayoutBinding

#include "02_OuterCore/Color32.h"         // for Color32
#include "02_OuterCore/FixedString.h"     // for FixedString
#include "02_OuterCore/VuConfig.h"        // for MAX_FRAMES_IN_FLIGHT, MATE...
#include "03_Mantle/VuCommon.h"           // for CommandBuffer, throw_if_un...
#include "03_Mantle/VuDevice.h"           // for VuDevice
#include "03_Mantle/VuGraphicsPipeline.h" // for VuGraphicsPipeline
#include "03_Mantle/VuPhysicalDevice.h"   // for VuPhysicalDevice, VuQueueF...
#include "03_Mantle/VuSampler.h"          // for VuSampler
#include "03_Mantle/VuSwapChain2.h"       // for VuSwapChain2
#include "04_Crust/VuShader.h"            // for VuShader
#include "imgui.h"                        // for GetDrawData, NewFrame, Render
#include "imgui_impl_sdl3.h"              // for ImGui_ImplSDL3_NewFrame
#include "imgui_impl_vulkan.h"            // for ImGui_ImplVulkan_NewFrame
#include "SDL3/SDL_events.h"              // for SDL_PollEvent, SDL_WaitEvent
#include "SDL3/SDL_mouse.h"               // for SDL_GetMouseState
#include "SDL3/SDL_timer.h"               // for SDL_GetTicksNS, SDL_GetTicks
#include "SDL3/SDL_video.h"               // for SDL_GetWindowFlags, SDL_Ge...
#include "stb_image.h"                    // for stbi_image_free, stbi_uc
#include "VuMaterial.h"                   // for VuMaterial, MaterialSettings
#include "VuMesh.h"                       // for VuMesh

namespace Vu {
struct VuMaterial;
}

namespace Vu {
bool
VuRenderer::shouldWindowClose() const {
  return sdlEvent.type == SDL_EVENT_QUIT;
}

void
VuRenderer::waitForFences() const {
  auto waitRes = vuDevice->device.waitForFences(*inFlightFences[currentFrame], vk::True, UINT64_MAX);
  if (waitRes != vk::Result::eSuccess) { throw std::runtime_error("VuRenderer::waitForFences: vk::Result failed"); }
}

void
VuRenderer::beginFrame() {
  waitForFences();
  std::pair<vk::Result, uint32_t> resultAndImageIndex = deferredRenderSpace.vuSwapChain.swapchain.acquireNextImage(
      UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr);
  currentFrameImageIndex = resultAndImageIndex.second;

  if (resultAndImageIndex.first == vk::Result::eErrorOutOfDateKHR) {
    resetSwapChain();
    std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
  } else if (resultAndImageIndex.first != vk::Result::eSuccess &&
             resultAndImageIndex.first != vk::Result::eSuboptimalKHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }
  // todo no return ?
  vuDevice->device.resetFences(*inFlightFences[currentFrame]);

  // vkResetFences(vuDevice.device, 1, &inFlightFences[currentFrame]);
  commandBuffers[currentFrame].reset();
  // vkResetCommandBuffer(commandBuffers[currentFrame], 0);

  vk::CommandBufferBeginInfo beginInfo {};

  // todo no ret ?
  commandBuffers[currentFrame].begin(beginInfo);
  // vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo);
  deferredRenderSpace.beginGBufferPass(commandBuffers[currentFrame], currentFrameImageIndex);
  // swapChain.beginGBufferPass(commandBuffers[currentFrame], currentFrameImageIndex);

  vk::Viewport viewport {};
  viewport.x        = 0.0f;
  viewport.y        = (float)deferredRenderSpace.vuSwapChain.extend2D.height;
  viewport.width    = (float)deferredRenderSpace.vuSwapChain.extend2D.width;
  viewport.height   = -(float)deferredRenderSpace.vuSwapChain.extend2D.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  commandBuffers[currentFrame].setViewport(0, viewport);
  // vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

  vk::Rect2D scissor {};
  scissor.offset = vk::Offset2D {0, 0};
  scissor.extent = deferredRenderSpace.vuSwapChain.extend2D;
  commandBuffers[currentFrame].setScissor(0, scissor);
  // vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
  bindGlobalBindlessSet(commandBuffers[currentFrame]);
}

void
VuRenderer::beginLightningPass() const {
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

void
VuRenderer::endFrame() {
  vk::raii::CommandBuffer& cb = commandBuffers[currentFrame];
  cb.endRenderPass();
  cb.end();

  vk::Semaphore          waitSemaphores[]   = {imageAvailableSemaphores[currentFrame]};
  vk::PipelineStageFlags waitStages[]       = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
  vk::Semaphore          signalSemaphores[] = {renderFinishedSemaphores[currentFrameImageIndex]};

  vk::SubmitInfo submitInfo {};
  submitInfo.pNext                = VK_NULL_HANDLE;
  submitInfo.waitSemaphoreCount   = 1u;
  submitInfo.pWaitSemaphores      = waitSemaphores;
  submitInfo.pWaitDstStageMask    = waitStages;
  submitInfo.commandBufferCount   = 1u;
  submitInfo.pCommandBuffers      = &*commandBuffers[currentFrame];
  submitInfo.signalSemaphoreCount = 1u;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  vuDevice->graphicsQueue.submit(submitInfo, inFlightFences[currentFrame]);

  // vk::Check(vkQueueSubmit(vuDevice.graphicsQueue, 1u, &submitInfo, inFlightFences[currentFrame]));

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

void
VuRenderer::bindMesh(VuMesh& mesh) {
  // we are using vertex pulling, so only index buffers we need to bind
  auto& commandBuffer = commandBuffers[currentFrame];
  auto  indexBuffer   = mesh.indexBuffer.get();
  commandBuffer.bindIndexBuffer(*indexBuffer->buffer, 0, vk::IndexType::eUint32);
}

void
VuRenderer::bindMaterial(std::shared_ptr<VuMaterial>& material) {
  auto& commandBuffer = commandBuffers[currentFrame];
  bindMaterial(commandBuffer, material);
}

void
VuRenderer::drawIndexed(u32 indexCount) const {
  auto& commandBuffer = commandBuffers[currentFrame];
  commandBuffer.drawIndexed(indexCount, 1, 0, 0, 0);
}

void
VuRenderer::pushConstants(const GPU_PushConstant& pushConstant) {
  auto& commandBuffer = commandBuffers[currentFrame];
  commandBuffer.pushConstants<GPU_PushConstant>(globalPipelineLayout, vk::ShaderStageFlagBits::eAll, 0, pushConstant);
  // vkCmdPushConstants(commandBuffer, vuDevice.globalPipelineLayout, VK_SHADER_STAGE_ALL, 0, config::PUSH_CONST_SIZE,
  //                    &pushConstant);
}

void
VuRenderer::beginImgui() const {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplSDL3_ProcessEvent(&sdlEvent);
  ImGui::NewFrame();
}

void
VuRenderer::endImgui() {
  auto& commandBuffer = commandBuffers[currentFrame];

  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), *commandBuffer);
}

void
VuRenderer::updateFrameConstantBuffer(GPU_FrameConst ubo) const {
  auto res = uniformBuffers[currentFrame].setData(&ubo, sizeof(ubo));
}

void
VuRenderer::resetSwapChain() {
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
}
void
VuRenderer::bindGlobalBindlessSet(const vk::CommandBuffer& commandBuffer) const {

  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, globalPipelineLayout, 0, 1,
                                   &globalDescriptorSets[currentFrame], 0, nullptr);
}
void
VuRenderer::PreUpdate() {
  // nano => micro => mili => second
  SDL_PollEvent(&sdlEvent);
  deltaAsSecond        = (SDL_GetTicksNS() - prevTimeAsNanoSecond) / 1000.0f / 1000.0f / 1000.0f;
  prevTimeAsNanoSecond = SDL_GetTicksNS();
}
void
VuRenderer::UpdateInput() {
  // SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
  float prevx = mouseX;
  float prevy = mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  mouseDeltaX = prevx - mouseX;
  mouseDeltaY = prevy - mouseY;
}
float
VuRenderer::time() {
  return SDL_GetTicks() / 1000.0f;
}
void
VuRenderer::writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const {
  vk::DescriptorBufferInfo bufferInfo {};
  bufferInfo.buffer = buffer.buffer;
  bufferInfo.offset = 0;
  bufferInfo.range  = sizeof(GPU_FrameConst);

  vk::WriteDescriptorSet descriptorWrite {};
  descriptorWrite.dstSet          = globalDescriptorSets[setIndex];
  descriptorWrite.dstBinding      = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType  = vk::DescriptorType::eUniformBuffer;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo     = &bufferInfo;
  vuDevice->device.updateDescriptorSets(descriptorWrite, {});
}
void
VuRenderer::registerToBindless(const VuBuffer& buffer, u32 bindlessIndex) const {
  vk::DeviceAddress address = buffer.getDeviceAddress();
  // TODO handle error
  auto res = bdaBuffer.setData(&address, sizeof(vk::DeviceAddress), bindlessIndex * sizeof(vk::DeviceAddress));
  if (res != vk::Result::eSuccess) { throw std::runtime_error("failed to set buffer data"); }
}
void
VuRenderer::registerToBindless(const vk::ImageView& imageView, u32 bindlessIndex) const {
  vk::DescriptorImageInfo imageInfo {};
  imageInfo.sampler     = nullptr;
  imageInfo.imageView   = imageView;
  imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    vk::WriteDescriptorSet descriptorWrite {};
    descriptorWrite.dstSet          = globalDescriptorSets[i];
    descriptorWrite.dstBinding      = lastCreateInfo.samplerBinding;
    descriptorWrite.dstArrayElement = bindlessIndex;
    descriptorWrite.descriptorType  = vk::DescriptorType::eSampledImage;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo      = &imageInfo;
    vuDevice->device.updateDescriptorSets(descriptorWrite, {});
  }
}
void
VuRenderer::registerToBindless(const vk::Sampler& sampler, u32 bindlessIndex) const {
  vk::DescriptorImageInfo imageInfo {
      .sampler = sampler,
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
}
void
VuRenderer::uninit() {
  disposeStack.disposeAll();
}
void
VuRenderer::initCommandPool(const VuRendererCreateInfo& info) {
  vk::CommandPoolCreateInfo poolInfo {};
  poolInfo.flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
  poolInfo.queueFamilyIndex = vuPhysicalDevice->indices.graphicsFamily.value();
  auto commandPoolOrErr     = vuDevice->device.createCommandPool(poolInfo);
  // Todo
  throw_if_unexpected(commandPoolOrErr);
  commandPool = std::move(commandPoolOrErr.value());
}
void
VuRenderer::initPipelineLayout() {
  std::array descSetLayouts {*globalDescriptorSetLayout};
  auto       pipelineLayoutOrErr = vuDevice->createPipelineLayout(descSetLayouts, config::PUSH_CONST_SIZE);
  throw_if_unexpected(pipelineLayoutOrErr);
  globalPipelineLayout = std::move(pipelineLayoutOrErr.value());
}
void
VuRenderer::initBindlessDescriptorSetLayout(const VuRendererCreateInfo& info) {
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
      ubo, sampler, sampledImage, storageImage, storageBuffer,
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
void
VuRenderer::initDefaultResources() {

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
  this->debugBufferHnd = std::make_shared<VuBuffer>(std::move(debugBufferOrErr.value()));

  // todo register bindless

  // assert(debugBufferHnd.index == 0);
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
  defaultImageHandle = std::make_shared<VuImage>(std::move(defaultImageOrErr.value()));
  uploadToImage(*defaultImageHandle.get(), reinterpret_cast<const byte*>(colorData.data()),
                colorData.size() * sizeof(Color32));
  // assert(defaultImageHandle.index == 0);
  // todo register

  Color32 normalColor = Color32(uint8_t(128), uint8_t(128), uint8_t(255), uint8_t(255));
  std::fill(colorData.begin(), colorData.end(), normalColor);
  auto defaultNormalImageOrErr =
      VuImage::make(vuDevice, {.width = 512, .height = 512, .format = vk::Format::eR8G8B8A8Unorm});
  throw_if_unexpected(defaultNormalImageOrErr);
  defaultNormalImageHandle = std::make_shared<VuImage>(std::move(defaultNormalImageOrErr.value()));

  uploadToImage(*defaultNormalImageHandle.get(), reinterpret_cast<const byte*>(colorData.data()),
                colorData.size() * sizeof(Color32));
  // assert(defaultNormalImageHandle.index == 1);
  // todo register

  // TODO pass physical props max

  auto defaultSamplerOrErr = VuSampler::make(vuDevice, {.maxAnisotropy = 16.0f});
  throw_if_unexpected(defaultSamplerOrErr);
  defaultSamplerHandle = std::make_shared<VuSampler>(std::move(defaultSamplerOrErr.value()));
  // assert(defaultSamplerHandle.get(). == 0);
  // todo register

  VuBufferCreateInfo matDataBufferCreateInfo {};
  matDataBufferCreateInfo.name        = "materialDataBuffer";
  matDataBufferCreateInfo.sizeInBytes = 4096;
  matDataBufferCreateInfo.vkUsageFlags =
      vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress;

  auto matDatBufferOrrErr = VuBuffer::make(*vuDevice, matDataBufferCreateInfo);
  throw_if_unexpected(matDatBufferOrrErr);
  this->materialDataBufferHandle = std::make_shared<VuBuffer>(std::move(matDatBufferOrrErr.value()));
  materialDataBufferHandle->map();

  // materialDataBufferHandle = createBuffer({
  //     .name           = "materialDataBuffer",
  //     .length         = config::DEVICE_MAX_MATERIAL_DATA_COUNT,
  //     .strideInBytes  = config::MATERIAL_DATA_SIZE,
  //     .vkUsageFlags   = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
  //     .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
  //     .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
  // });

  // todo register to bindless
  // assert(materialDataBufferHandle.get()->bindlessIndex == 1);
  //  VuBuffer* matDataBuffer = materialDataBufferHandle.get();
  //  matDataBuffer->map();
}
void
VuRenderer::initDescriptorPool(const VuRendererCreateInfo& info) {
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
void
VuRenderer::initBindlessDescriptorSet() {
  std::array<vk::DescriptorSetLayout, config::MAX_FRAMES_IN_FLIGHT> globalDescLayout;
  globalDescLayout.fill(globalDescriptorSetLayout);

  vk::DescriptorSetAllocateInfo globalSetsAllocInfo;
  globalSetsAllocInfo.descriptorPool     = descriptorPool;
  globalSetsAllocInfo.descriptorSetCount = static_cast<uint32_t>(globalDescLayout.size());
  globalSetsAllocInfo.pSetLayouts        = globalDescLayout.data();

  std::array<VkDescriptorSet, config::MAX_FRAMES_IN_FLIGHT> tempDescSets;
  vkAllocateDescriptorSets(*vuDevice->device, globalSetsAllocInfo, tempDescSets.data());
  globalDescriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
  for (uint32_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    globalDescriptorSets[i] = vk::DescriptorSet {tempDescSets[i]};
  }
  // auto descSetsOrErr = vuDevice->device.allocateDescriptorSets(globalSetsAllocInfo);
  // throw_if_unexpected(descSetsOrErr);

  // TODO

  // for (vk::raii::DescriptorSet descSet : descSetsOrErr.value()) {
  //   globalDescriptorSets.push_back(descSet);
  // }
}
vk::raii::CommandBuffer
VuRenderer::BeginSingleTimeCommands() const {
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
void
VuRenderer::EndSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const {
  commandBuffer.end();

  vk::SubmitInfo submitInfo;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &*commandBuffer;

  vuDevice->graphicsQueue.submit(submitInfo);
  vuDevice->graphicsQueue.waitIdle();

  // device.freeCommandBuffers(commandPool, 1, &commandBuffer);
}
void
VuRenderer::initBindlessResourceManager(const VuRendererCreateInfo& info) {

  VuBufferCreateInfo bufferCreateInfo {
      .name        = "BDA_Buffer",
      .sizeInBytes = info.storageBufferCount * sizeof(vk::DeviceAddress),
  };
  auto bdaBufferOrErr = VuBuffer::make(*vuDevice, bufferCreateInfo);
  throw_if_unexpected(bdaBufferOrErr);
  bdaBuffer = std::move(bdaBufferOrErr.value());

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
std::shared_ptr<Vu::VuImage>
VuRenderer::createImageFromAsset(const path& path, vk::Format format) {
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
  std::shared_ptr<VuImage> vuImage = std::make_shared<VuImage>(std::move(vuImageOrrErr.value()));
  uploadToImage(*vuImage, pixels, imageSize);
  stbi_image_free(pixels);
  return vuImage;
}
std::shared_ptr<VuMaterialDataHandle>
VuRenderer::createMaterialDataIndex() {
  std::shared_ptr<VuMaterialDataHandle> handle        = std::make_shared<VuMaterialDataHandle>();
  VuMaterialDataHandle*                 resource      = handle.get();
  u32                                   bindlessIndex = materialDataBindlessIndexAllocator.allocate();
  resource->index                                     = bindlessIndex;
  return handle;
}
std::span<std::byte, Vu::config::MATERIAL_DATA_SIZE>
VuRenderer::getMaterialData(const std::shared_ptr<VuMaterialDataHandle>& handle) const {
  byte* dataPtr = &static_cast<byte*>(materialDataBufferHandle->mapPtr)[config::MATERIAL_DATA_SIZE * handle->index];
  return std::span<std::byte, config::MATERIAL_DATA_SIZE>(dataPtr, config::MATERIAL_DATA_SIZE);
}
void
VuRenderer::bindMaterial(const vk::CommandBuffer& cb, const std::shared_ptr<VuMaterial>& material) {
  VuMaterial*         mat        = material.get();
  VuShader*           shader     = mat->shaderHnd.get();
  VuGraphicsPipeline& vuPipeline = shader->requestPipeline(mat->materialSettings);

  cb.bindPipeline(vk::PipelineBindPoint::eGraphics, vuPipeline.pipeline);
  // vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.pipeline);
}
void
VuRenderer::copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size) {
  vk::raii::CommandBuffer commandBuffer = BeginSingleTimeCommands();
  vk::BufferCopy          copyRegion {};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size      = size;
  commandBuffer.copyBuffer(srcBuffer, dstBuffer, copyRegion);
  EndSingleTimeCommands(commandBuffer);
}
void
VuRenderer::uploadToImage(const VuImage& vuImage, const byte* data, const vk::DeviceSize size) {
  // todo
  auto res = stagingBuffer.setData(data, size);

  transitionImageLayout(vuImage.image, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

  copyBufferToImage(stagingBuffer.buffer, vuImage.image, vuImage.lastCreateInfo.width, vuImage.lastCreateInfo.height);

  transitionImageLayout(vuImage.image, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
void
VuRenderer::transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) {
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
void
VuRenderer::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height) {
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

} // namespace Vu
