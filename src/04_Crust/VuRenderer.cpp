#include "VuRenderer.h"

#include <algorithm> // for fill
#include <array>     // for array
#include <assert.h>
#include <expected> // for expected
#include <functional>
#include <iostream> // for char_traits, basic_ostream
#include <memory_resource>
#include <queue>
#include <stdexcept> // for runtime_error, invalid_arg...
#include <utility>   // for move, pair
#include <vector>    // for vector

#include "01_InnerCore/ScopeTimer.h"
#include "02_OuterCore/Color32.h"     // for Color32
#include "02_OuterCore/FixedString.h" // for FixedString
#include "02_OuterCore/VuCommon.h"
#include "02_OuterCore/VuConfig.h"        // for MAX_FRAMES_IN_FLIGHT, MATE...
#include "03_Mantle/VuDevice.h"           // for VuDevice
#include "03_Mantle/VuGraphicsPipeline.h" // for VuGraphicsPipeline
#include "03_Mantle/VuImage.h"
#include "03_Mantle/VuInstance.h"
#include "03_Mantle/VuPhysicalDevice.h" // for VuPhysicalDevice, VuQueueF...
#include "03_Mantle/VuRenderPass.h"
#include "03_Mantle/VuSampler.h"   // for VuSampler
#include "03_Mantle/VuSwapChain.h" // for VuSwapChain2
#include "04_Crust/VuDeferredRenderSpace.h"
#include "04_Crust/VuShader.h" // for VuShader
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

VuRenderer::VuRenderer(const VuRendererCreateInfo& createInfo) :
    m_imgBindlessIndexAllocator {createInfo.sampledImageCount, std::pmr::new_delete_resource()},
    m_samplerBindlessIndexAllocator {createInfo.samplerCount, std::pmr::new_delete_resource()},
    m_bufferBindlessIndexAllocator {createInfo.storageBufferCount, std::pmr::new_delete_resource()},
    m_materialDataBindlessIndexAllocator {1024, std::pmr::new_delete_resource()},
    m_lastCreateInfo {createInfo} {
  ScopeTimer timer;
  bool       isValidationEnabled = config::ENABLE_VALIDATION_LAYERS_LAYERS;

  // window
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);

  this->m_window = SDL_CreateWindow(
      "VuRenderer", Vu::config::START_WIDTH, Vu::config::START_HEIGHT, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  // instance extensions
  u32                count                          = 0;
  const char* const* sdlRequestedInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&count);

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
  THROW_if_unexpected(instanceOrErr);
  this->m_vuInstance = std::make_shared<VuInstance>(std::move(instanceOrErr.value()));

  // surface
  VkSurfaceKHR tempSurface = nullptr;
  bool surfaceResult = SDL_Vulkan_CreateSurface(m_window, m_vuInstance->m_instance, NO_ALLOC_CALLBACK, &tempSurface);
  assert(surfaceResult == true);

  // instance
  this->m_surface = std::make_shared<VkSurfaceKHR>(tempSurface);

  // phyDevice
  auto phyDevOrErr = Vu::VuPhysicalDevice::make(m_vuInstance, tempSurface, Vu::config::DEVICE_EXTENSIONS);
  THROW_if_unexpected(phyDevOrErr);
  this->m_vuPhysicalDevice = std::make_shared<VuPhysicalDevice>(std::move(phyDevOrErr.value()));

  // device
  VuDeviceCreateFeatureChain defaultFeatureChain {};

  auto vuDeviceOrErr =
      VuDevice::make(m_vuPhysicalDevice, defaultFeatureChain.deviceFeatures2, config::DEVICE_EXTENSIONS);
  THROW_if_unexpected(vuDeviceOrErr);
  this->m_vuDevice = std::make_shared<VuDevice>(std::move(vuDeviceOrErr.value()));

  initCommandPool(createInfo);
  initBindlessDescriptorSetLayout(createInfo);
  initDescriptorPool(createInfo);
  initPipelineLayout();
  initBindlessDescriptorSet();
  initBindlessResourceManager(createInfo);
  initDefaultResources();

  // init uniform buffers

  m_uniformBuffers.clear();
  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    VkDeviceSize bufferSize = sizeof(m_frameConst);

    // uniformBuffers[i] = VuBuffer {nullptr};
    VuBufferCreateInfo bufferCreateInfo {
        .name         = "UniformBuffer",
        .sizeInBytes  = bufferSize,
        .vkUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };
    m_uniformBuffers.emplace_back(move_or_THROW(VuBuffer::make(m_vuDevice, bufferCreateInfo)));
    THROW_if_fail(m_uniformBuffers[i].map());
  }
  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    writeUBO_ToGlobalPool(m_uniformBuffers[i], 0, i);
  }

  // init command buffers

  VkCommandBufferAllocateInfo allocInfo {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  allocInfo.commandPool        = m_commandPool;
  allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = static_cast<u32>(config::MAX_FRAMES_IN_FLIGHT);

  m_commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
  VkResult cmdBuffersRes = vkAllocateCommandBuffers(m_vuDevice->m_device, &allocInfo, m_commandBuffers.data());
  THROW_if_fail(cmdBuffersRes);

  VuDeferredRenderSpace rp {m_vuDevice, m_surface};
  this->m_deferredRenderSpace = std::move(rp);
  m_deferredRenderSpace.registerImagesToBindless(*this);

  // init sync objects
  VkSemaphoreCreateInfo semaphoreInfo {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
  VkFenceCreateInfo     fenceInfo {.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};

  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  uint32_t swapChainImageCount = m_deferredRenderSpace.m_vuSwapChain.m_images.size();
  m_imageAvailableSemaphores.resize(swapChainImageCount);
  m_renderFinishedSemaphores.resize(swapChainImageCount);
  m_inFlightFences.resize(swapChainImageCount);
  for (size_t i = 0; i < swapChainImageCount; i++) {
    THROW_if_fail(
        vkCreateSemaphore(m_vuDevice->m_device, &semaphoreInfo, NO_ALLOC_CALLBACK, &m_imageAvailableSemaphores[i]));
    THROW_if_fail(
        vkCreateSemaphore(m_vuDevice->m_device, &semaphoreInfo, NO_ALLOC_CALLBACK, &m_renderFinishedSemaphores[i]));
    THROW_if_fail(vkCreateFence(m_vuDevice->m_device, &fenceInfo, NO_ALLOC_CALLBACK, &m_inFlightFences[i]));
  }

  initImGui();
}

bool
VuRenderer::shouldWindowClose() const {
  return m_sdlEvent.type == SDL_EVENT_QUIT;
}

void
VuRenderer::waitForFences() const {
  THROW_if_fail(vkWaitForFences(m_vuDevice->m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX));
}

void
VuRenderer::beginFrame() {
  waitForFences();

  uint32_t swapChainImageIndex {};
  VkResult imageIndexRes = vkAcquireNextImageKHR(m_vuDevice->m_device,
                                                 m_deferredRenderSpace.m_vuSwapChain.m_swapchain,
                                                 UINT64_MAX,
                                                 m_imageAvailableSemaphores[m_currentFrame],
                                                 nullptr,
                                                 &swapChainImageIndex);
  // std::pair<VkResult, uint32_t> resultAndImageIndex = deferredRenderSpace.vuSwapChain.swapchain.acquireNextImage(
  //     UINT64_MAX, imageAvailableSemaphores[currentFrame], nullptr);

  if (imageIndexRes == VK_ERROR_OUT_OF_DATE_KHR) {
    resetSwapChain();
    std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
  } else if (imageIndexRes != VK_SUCCESS && imageIndexRes != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire swap chain image!");
  }

  if (imageIndexRes != VK_SUCCESS) { throw std::runtime_error("VuRenderer::beginFrame: swapchain acquire failed"); }
  m_currentFrameImageIndex = swapChainImageIndex;

  THROW_if_fail(vkResetFences(m_vuDevice->m_device, 1, &m_inFlightFences[m_currentFrame]));
  THROW_if_fail(vkResetCommandBuffer(m_commandBuffers[m_currentFrame], ZERO_FLAG));

  VkCommandBufferBeginInfo beginInfo {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};

  THROW_if_fail(vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo));

  m_deferredRenderSpace.beginGBufferPass(m_commandBuffers[m_currentFrame], m_currentFrameImageIndex);

  VkViewport viewport {};
  viewport.x        = 0.0f;
  viewport.y        = (float)m_deferredRenderSpace.m_vuSwapChain.m_extend2D.height;
  viewport.width    = (float)m_deferredRenderSpace.m_vuSwapChain.m_extend2D.width;
  viewport.height   = -(float)m_deferredRenderSpace.m_vuSwapChain.m_extend2D.height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(m_commandBuffers[m_currentFrame], 0, 1, &viewport);

  VkRect2D scissor {};
  scissor.offset = VkOffset2D {0, 0};
  scissor.extent = m_deferredRenderSpace.m_vuSwapChain.m_extend2D;
  vkCmdSetScissor(m_commandBuffers[m_currentFrame], 0, 1, &scissor);
  bindGlobalBindlessSet(m_commandBuffers[m_currentFrame]);
}

void
VuRenderer::beginLightningPass() const {
  const VkCommandBuffer& cb = m_commandBuffers[m_currentFrame];
  vkCmdEndRenderPass(cb);

  VkPipelineStageFlags srcStage =
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;

  VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

  VkMemoryBarrier memoryBarrier = {.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER};

  memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

  memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

  vkCmdPipelineBarrier(cb, srcStage, dstStage, ZERO_FLAG, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

  // lightning pass
  m_deferredRenderSpace.beginLightningPass(m_commandBuffers[m_currentFrame], m_currentFrameImageIndex);
}

void
VuRenderer::endFrame() {
  const VkCommandBuffer& cb = m_commandBuffers[m_currentFrame];
  vkCmdEndRenderPass(cb);
  THROW_if_fail(vkEndCommandBuffer(cb));

  VkSemaphore          waitSemaphores[]   = {m_imageAvailableSemaphores[m_currentFrame]};
  VkPipelineStageFlags waitStages[]       = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  VkSemaphore          signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};

  VkSubmitInfo submitInfo {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.waitSemaphoreCount   = 1u;
  submitInfo.pWaitSemaphores      = waitSemaphores;
  submitInfo.pWaitDstStageMask    = waitStages;
  submitInfo.commandBufferCount   = 1u;
  submitInfo.pCommandBuffers      = &m_commandBuffers[m_currentFrame];
  submitInfo.signalSemaphoreCount = 1u;
  submitInfo.pSignalSemaphores    = signalSemaphores;

  THROW_if_fail(vkQueueSubmit(m_vuDevice->m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]));

  VkSwapchainKHR swapChains[] = {m_deferredRenderSpace.m_vuSwapChain.m_swapchain};

  VkPresentInfoKHR presentInfo {.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.pNext              = nullptr;
  presentInfo.waitSemaphoreCount = 1u;
  presentInfo.pWaitSemaphores    = signalSemaphores;
  presentInfo.swapchainCount     = 1u;
  presentInfo.pSwapchains        = swapChains;
  presentInfo.pImageIndices      = &m_currentFrameImageIndex;
  presentInfo.pResults           = VK_NULL_HANDLE;

  VkResult presentRes = vkQueuePresentKHR(m_vuDevice->m_presentQueue, &presentInfo);

  if (presentRes == VK_ERROR_OUT_OF_DATE_KHR || presentRes == VK_SUBOPTIMAL_KHR) {
    resetSwapChain();
  } else if (presentRes != VK_SUCCESS) {
    throw std::runtime_error("failed to present swap chain image!");
  }
  m_currentFrame = (m_currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
}

void
VuRenderer::bindMesh(VuMesh& mesh) {
  // we are using vertex pulling, so only index buffers we need to bind
  auto& commandBuffer = m_commandBuffers[m_currentFrame];
  auto  indexBuffer   = mesh.m_indexBuffer.get();
  vkCmdBindIndexBuffer(commandBuffer, indexBuffer->m_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void
VuRenderer::bindMaterial(std::shared_ptr<VuMaterial>& material) {
  auto& commandBuffer = m_commandBuffers[m_currentFrame];
  bindMaterial(commandBuffer, material);
}

void
VuRenderer::drawIndexed(u32 indexCount) const {
  auto& commandBuffer = m_commandBuffers[m_currentFrame];
  vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void
VuRenderer::pushConstants(const PushConsts_RawData& pushConstant) {
  auto& commandBuffer = m_commandBuffers[m_currentFrame];
  vkCmdPushConstants(commandBuffer,
                     m_globalPipelineLayout,
                     VK_SHADER_STAGE_ALL,
                     MakeVkOffset(0),
                     config::PUSH_CONST_SIZE,
                     &pushConstant);
}

void
VuRenderer::beginImgui() const {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplSDL3_NewFrame();
  ImGui_ImplSDL3_ProcessEvent(&m_sdlEvent);
  ImGui::NewFrame();
}

void
VuRenderer::endImgui() const {
  auto& commandBuffer = m_commandBuffers[m_currentFrame];

  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void
VuRenderer::updateFrameConstantBuffer(FrameConst_RawData ubo) const {
  auto res = m_uniformBuffers[m_currentFrame].setData(&ubo, sizeof(ubo));
  if (res != VK_SUCCESS) { throw std::runtime_error("failed to update frame constant buffer!"); }
}

void
VuRenderer::resetSwapChain() {
  SDL_Event event;

  int width  = 0;
  int height = 0;
  SDL_GetWindowSize(m_window, &width, &height);
  auto minimized = (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;

  while (width <= 0 || height <= 0 || minimized) {
    SDL_GetWindowSize(m_window, &width, &height);
    minimized = (SDL_GetWindowFlags(m_window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
    SDL_WaitEvent(&event);
  }
  THROW_if_fail(vkDeviceWaitIdle(m_vuDevice->m_device));

  VuDeferredRenderSpace rp {m_vuDevice, m_surface};
  this->m_deferredRenderSpace = std::move(rp);
  m_deferredRenderSpace.registerImagesToBindless(*this);
}
void
VuRenderer::bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer) const {

  vkCmdBindDescriptorSets(commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_globalPipelineLayout,
                          0,
                          1,
                          &m_globalDescriptorSets[m_currentFrame],
                          0,
                          nullptr);
}
void
VuRenderer::preUpdate() {
  // nano => micro => mili => second
  SDL_PollEvent(&m_sdlEvent);
  m_deltaAsSecond        = (SDL_GetTicksNS() - m_prevTimeAsNanoSecond) / 1000.0f / 1000.0f / 1000.0f;
  m_prevTimeAsNanoSecond = SDL_GetTicksNS();
}
void
VuRenderer::pollUserInput() {
  // SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
  float prevx = m_mouseX;
  float prevy = m_mouseY;
  SDL_GetMouseState(&m_mouseX, &m_mouseY);
  m_mouseDeltaX = prevx - m_mouseX;
  m_mouseDeltaY = prevy - m_mouseY;
}
float
VuRenderer::time() {
  return SDL_GetTicks() / 1000.0f;
}
void
VuRenderer::writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const {
  VkDescriptorBufferInfo bufferInfo {};
  bufferInfo.buffer = buffer.m_buffer;
  bufferInfo.offset = 0;
  bufferInfo.range  = sizeof(FrameConst_RawData);

  VkWriteDescriptorSet descriptorWrite {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
  descriptorWrite.dstSet          = m_globalDescriptorSets[setIndex];
  descriptorWrite.dstBinding      = 0;
  descriptorWrite.dstArrayElement = 0;
  descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  descriptorWrite.descriptorCount = 1;
  descriptorWrite.pBufferInfo     = &bufferInfo;
  vkUpdateDescriptorSets(m_vuDevice->m_device, 1, &descriptorWrite, 0, nullptr);
}
void
VuRenderer::registerToBindless(VuBuffer& vuBuffer) {
  uint32_t        bindlessIndex = m_bufferBindlessIndexAllocator.allocate();
  VkDeviceAddress address       = vuBuffer.getDeviceAddress();
  // TODO handle error
  // auto view                       = std::span((uint64_t*)bdaBuffer.mapPtr, bdaBuffer.sizeInBytes / 64);
  auto res = m_bdaBuffer.setData(&address, sizeof(VkDeviceAddress), bindlessIndex * sizeof(VkDeviceAddress));
  if (res != VK_SUCCESS) { throw std::runtime_error("failed to set buffer data"); }
  vuBuffer.m_bindlessIndex = bindlessIndex;
}
void
VuRenderer::registerToBindless(VuImage& vuImage) {
  uint32_t bindlessIndex = m_imgBindlessIndexAllocator.allocate();

  VkDescriptorImageInfo imageInfo {};
  imageInfo.sampler     = nullptr;
  imageInfo.imageView   = vuImage.m_imageView;
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    VkWriteDescriptorSet descriptorWrite {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstSet          = m_globalDescriptorSets[i];
    descriptorWrite.dstBinding      = m_lastCreateInfo.sampledImageBinding;
    descriptorWrite.dstArrayElement = bindlessIndex;
    descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo      = &imageInfo;
    vkUpdateDescriptorSets(m_vuDevice->m_device, 1, &descriptorWrite, 0, nullptr);
  }
  vuImage.m_bindlessIndex = bindlessIndex;
}
void
VuRenderer::registerToBindless(VuSampler& vuSampler) {
  uint32_t              bindlessIndex = m_samplerBindlessIndexAllocator.allocate();
  VkDescriptorImageInfo imageInfo {
      .sampler = vuSampler.m_sampler,
  };

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    VkWriteDescriptorSet descriptorWrite {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstSet          = m_globalDescriptorSets[i];
    descriptorWrite.dstBinding      = m_lastCreateInfo.samplerBinding;
    descriptorWrite.dstArrayElement = bindlessIndex;
    descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo      = &imageInfo;
    vkUpdateDescriptorSets(m_vuDevice->m_device, 1, &descriptorWrite, 0, nullptr);
  }
  vuSampler.m_bindlessIndex = bindlessIndex;
}
// void
// VuRenderer::uninit() {
//   m_disposeStack.disposeAll();
// }
void
VuRenderer::initCommandPool(const VuRendererCreateInfo& info) {
  VkCommandPoolCreateInfo poolInfo {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
  poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex = m_vuPhysicalDevice->m_indices.graphicsFamily;
  THROW_if_fail(vkCreateCommandPool(m_vuDevice->m_device, &poolInfo, NO_ALLOC_CALLBACK, &m_commandPool));
}
void
VuRenderer::initPipelineLayout() {
  std::array descSetLayouts {m_globalDescriptorSetLayout};
  auto       pipelineLayoutOrErr = m_vuDevice->createPipelineLayout(descSetLayouts, config::PUSH_CONST_SIZE);
  THROW_if_unexpected(pipelineLayoutOrErr);
  m_globalPipelineLayout = std::move(pipelineLayoutOrErr.value());
}
void
VuRenderer::initBindlessDescriptorSetLayout(const VuRendererCreateInfo& info) {
  VkDescriptorSetLayoutBinding ubo {};
  ubo.binding         = info.uboBinding;
  ubo.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  ubo.descriptorCount = info.uboCount;
  ubo.stageFlags      = VK_SHADER_STAGE_ALL;

  VkDescriptorSetLayoutBinding sampler {};
  sampler.binding         = info.samplerBinding;
  sampler.descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLER;
  sampler.descriptorCount = info.samplerCount;
  sampler.stageFlags      = VK_SHADER_STAGE_ALL;

  VkDescriptorSetLayoutBinding sampledImage = {};
  sampledImage.binding                      = info.sampledImageBinding;
  sampledImage.descriptorType               = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  sampledImage.descriptorCount              = info.sampledImageCount;
  sampledImage.stageFlags                   = VK_SHADER_STAGE_ALL;

  VkDescriptorSetLayoutBinding storageImage {};
  storageImage.binding         = info.storageImageBinding;
  storageImage.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  storageImage.descriptorCount = info.storageImageCount;
  storageImage.stageFlags      = VK_SHADER_STAGE_ALL;

  VkDescriptorSetLayoutBinding storageBuffer {};
  storageBuffer.binding         = info.storageBufferBinding;
  storageBuffer.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  storageBuffer.descriptorCount = 1;
  storageBuffer.stageFlags      = VK_SHADER_STAGE_ALL;

  std::array descriptorSetLayoutBindings {
      ubo,
      sampler,
      sampledImage,
      storageImage,
      storageBuffer,
  };

  VkDescriptorSetLayoutCreateInfo descSetLayoutCreateInfo {.sType =
                                                               VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

  descSetLayoutCreateInfo.flags        = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
  descSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
  descSetLayoutCreateInfo.pBindings    = descriptorSetLayoutBindings.data();

  constexpr VkDescriptorBindingFlagsEXT flag = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
                                               VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
                                               VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;

  std::array descriptorSetLayoutFlags {flag, flag, flag, flag, flag};

  VkDescriptorSetLayoutBindingFlagsCreateInfo binding_flags {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO};
  binding_flags.bindingCount  = descriptorSetLayoutFlags.size();
  binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();

  descSetLayoutCreateInfo.pNext = &binding_flags;

  VkResult layoutRes = vkCreateDescriptorSetLayout(
      m_vuDevice->m_device, &descSetLayoutCreateInfo, NO_ALLOC_CALLBACK, &m_globalDescriptorSetLayout);
  THROW_if_fail(layoutRes);
}
void
VuRenderer::initDefaultResources() {
  VuBufferCreateInfo stagingBufferCreateInfo {};
  stagingBufferCreateInfo.name         = "stagingBuffer";
  stagingBufferCreateInfo.sizeInBytes  = 1024 * 1024 * 64;
  stagingBufferCreateInfo.vkUsageFlags = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

  auto stagingOrErr = VuBuffer::make(m_vuDevice, stagingBufferCreateInfo);
  THROW_if_unexpected(stagingOrErr);
  this->m_stagingBuffer = std::move(stagingOrErr.value());
  THROW_if_fail(m_stagingBuffer.map());

  VuBufferCreateInfo debugBufferCreateInfo {};
  debugBufferCreateInfo.name         = "debugBuffer";
  debugBufferCreateInfo.sizeInBytes  = 4096;
  debugBufferCreateInfo.vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

  auto debugBufferOrErr = VuBuffer::make(m_vuDevice, debugBufferCreateInfo);
  THROW_if_unexpected(debugBufferOrErr);
  this->m_debugBuffer = std::make_shared<VuBuffer>(std::move(debugBufferOrErr.value()));

  registerToBindless(*m_debugBuffer);
  assert(m_debugBuffer->m_bindlessIndex == 0);

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

  auto defaultImageOrErr = VuImage::make(m_vuDevice, {.width = 512, .height = 512, .format = VK_FORMAT_R8G8B8A8_SRGB});
  THROW_if_unexpected(defaultImageOrErr);
  m_defaultImage = std::make_shared<VuImage>(std::move(defaultImageOrErr.value()));
  uploadToImage(*m_defaultImage, reinterpret_cast<const byte*>(colorData.data()), colorData.size() * sizeof(Color32));
  registerToBindless(*m_defaultImage);
  assert(m_defaultImage->m_bindlessIndex == 0);

  Color32 normalColor {uint8_t {128}, uint8_t {128}, uint8_t {255}, uint8_t {255}};
  std::fill(colorData.begin(), colorData.end(), normalColor);
  auto defaultNormalImageOrErr =
      VuImage::make(m_vuDevice, {.width = 512, .height = 512, .format = VK_FORMAT_R8G8B8A8_UNORM});
  THROW_if_unexpected(defaultNormalImageOrErr);
  m_defaultNormalImage = std::make_shared<VuImage>(std::move(defaultNormalImageOrErr.value()));

  uploadToImage(
      *m_defaultNormalImage, reinterpret_cast<const byte*>(colorData.data()), colorData.size() * sizeof(Color32));
  registerToBindless(*m_defaultNormalImage);
  assert(m_defaultNormalImage->m_bindlessIndex == 1);

  // TODO pass physical props max

  auto defaultSamplerOrErr = VuSampler::make(m_vuDevice, {.maxAnisotropy = 16.0f});
  THROW_if_unexpected(defaultSamplerOrErr);
  m_defaultSampler = std::make_shared<VuSampler>(std::move(defaultSamplerOrErr.value()));
  registerToBindless(*m_defaultSampler);
  assert(m_defaultSampler->m_bindlessIndex == 0);

  VuBufferCreateInfo matDataBufferCreateInfo {};
  matDataBufferCreateInfo.name         = "materialDataBuffer";
  matDataBufferCreateInfo.sizeInBytes  = 4096;
  matDataBufferCreateInfo.vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;

  auto matDatBufferOrrErr = VuBuffer::make(m_vuDevice, matDataBufferCreateInfo);
  THROW_if_unexpected(matDatBufferOrrErr);
  this->m_materialDataBuffer = std::make_shared<VuBuffer>(std::move(matDatBufferOrrErr.value()));
  registerToBindless(*m_materialDataBuffer);
  assert(m_materialDataBuffer->m_bindlessIndex == 1);
  THROW_if_fail(m_materialDataBuffer->map());
}
void
VuRenderer::initDescriptorPool(const VuRendererCreateInfo& info) {
  std::array<VkDescriptorPoolSize, 5> poolSizes;

  poolSizes[0].type            = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  poolSizes[0].descriptorCount = info.uboCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[1].type            = VK_DESCRIPTOR_TYPE_SAMPLER;
  poolSizes[1].descriptorCount = info.samplerCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[2].type            = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
  poolSizes[2].descriptorCount = info.sampledImageCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[3].type            = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  poolSizes[3].descriptorCount = info.storageImageCount * config::MAX_FRAMES_IN_FLIGHT;

  poolSizes[4].type            = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  poolSizes[4].descriptorCount = info.storageBufferCount * config::MAX_FRAMES_IN_FLIGHT;

  VkDescriptorPoolCreateInfo poolInfo {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
  poolInfo.maxSets       = 2;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes    = poolSizes.data();

  THROW_if_fail(vkCreateDescriptorPool(m_vuDevice->m_device, &poolInfo, NO_ALLOC_CALLBACK, &m_descriptorPool));
}
void
VuRenderer::initBindlessDescriptorSet() {
  std::array<VkDescriptorSetLayout, config::MAX_FRAMES_IN_FLIGHT> globalDescLayout;
  globalDescLayout.fill(m_globalDescriptorSetLayout);

  VkDescriptorSetAllocateInfo globalSetsAllocInfo {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
  globalSetsAllocInfo.descriptorPool     = m_descriptorPool;
  globalSetsAllocInfo.descriptorSetCount = static_cast<uint32_t>(globalDescLayout.size());
  globalSetsAllocInfo.pSetLayouts        = globalDescLayout.data();

  std::array<VkDescriptorSet, config::MAX_FRAMES_IN_FLIGHT> tempDescSets {};
  THROW_if_fail(vkAllocateDescriptorSets(m_vuDevice->m_device, &globalSetsAllocInfo, tempDescSets.data()));
  m_globalDescriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
  for (uint32_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    m_globalDescriptorSets[i] = VkDescriptorSet {tempDescSets[i]};
  }
}
VkCommandBuffer
VuRenderer::beginSingleTimeCommands() const {
  VkCommandBufferAllocateInfo cbAllocInfo {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
  cbAllocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cbAllocInfo.commandPool        = m_commandPool;
  cbAllocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer {};
  THROW_if_fail(vkAllocateCommandBuffers(m_vuDevice->m_device, &cbAllocInfo, &commandBuffer));

  VkCommandBufferBeginInfo beginInfo {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  THROW_if_fail(vkBeginCommandBuffer(commandBuffer, &beginInfo));
  return commandBuffer;
}
void
VuRenderer::endSingleTimeCommands(const VkCommandBuffer& commandBuffer) const {
  THROW_if_fail(vkEndCommandBuffer(commandBuffer));

  VkSubmitInfo submitInfo {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO};
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers    = &commandBuffer;

  THROW_if_fail(vkQueueSubmit(m_vuDevice->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
  THROW_if_fail(vkQueueWaitIdle(m_vuDevice->m_graphicsQueue));

  vkFreeCommandBuffers(m_vuDevice->m_device, m_commandPool, 1, &commandBuffer);
}
void
VuRenderer::initBindlessResourceManager(const VuRendererCreateInfo& info) {

  VuBufferCreateInfo bufferCreateInfo {
      .name        = "BDA_Buffer",
      .sizeInBytes = info.storageBufferCount * sizeof(VkDeviceAddress),
  };
  auto bdaBufferOrErr = VuBuffer::make(m_vuDevice, bufferCreateInfo);
  THROW_if_unexpected(bdaBufferOrErr);
  m_bdaBuffer = std::move(bdaBufferOrErr.value());
  THROW_if_fail(m_bdaBuffer.map());

  VkDescriptorBufferInfo descBufferInfo {};
  descBufferInfo.buffer = m_bdaBuffer.m_buffer;
  descBufferInfo.range  = m_bdaBuffer.m_sizeInBytes;

  for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
    VkWriteDescriptorSet descriptorWrite {.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    descriptorWrite.dstSet          = m_globalDescriptorSets[i];
    descriptorWrite.dstBinding      = m_lastCreateInfo.storageBufferBinding;
    descriptorWrite.dstArrayElement = 0;
    descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo     = &descBufferInfo;

    vkUpdateDescriptorSets(m_vuDevice->m_device, 1, &descriptorWrite, 0, nullptr);
  }
}
VuImage
VuRenderer::createImageFromAsset(const path& path, VkFormat format) {
  int   texWidth;
  int   texHeight;
  int   texChannels;
  byte* pixels;

  Vu::VuImage::loadImageFile(path, texWidth, texHeight, texChannels, reinterpret_cast<stbi_uc*&>(pixels));
  const auto imageSize = static_cast<VkDeviceSize>(texWidth * texHeight * 4U);

  if (pixels == nullptr) { throw std::runtime_error("failed to load texture image!"); }

  u32               w = texWidth;
  u32               h = texHeight;
  VuImageCreateInfo createInfo {.width = w, .height = h, .format = format};

  auto vuImageOrrErr = VuImage::make(m_vuDevice, createInfo);
  THROW_if_unexpected(vuImageOrrErr);
  uploadToImage(vuImageOrrErr.value(), pixels, imageSize);
  stbi_image_free(pixels);
  return std::move(vuImageOrrErr.value());
}

std::shared_ptr<VuMaterialDataHandle>
VuRenderer::createMaterialDataIndex() {
  std::shared_ptr<VuMaterialDataHandle> handle        = std::make_shared<VuMaterialDataHandle>();
  VuMaterialDataHandle*                 resource      = handle.get();
  u32                                   bindlessIndex = m_materialDataBindlessIndexAllocator.allocate();
  resource->index                                     = bindlessIndex;
  return handle;
}
std::span<std::byte, config::MATERIAL_DATA_SIZE>
VuRenderer::getMaterialDataSpan(const std::shared_ptr<VuMaterialDataHandle>& handle) const {
  byte* dataPtr = &static_cast<byte*>(m_materialDataBuffer->m_mapPtr)[config::MATERIAL_DATA_SIZE * handle->index];
  return std::span<std::byte, config::MATERIAL_DATA_SIZE>(dataPtr, config::MATERIAL_DATA_SIZE);
}

void
VuRenderer::bindMaterial(const VkCommandBuffer& cb, const std::shared_ptr<VuMaterial>& material) {
  VuMaterial*         mat        = material.get();
  VuShader*           shader     = mat->m_shaderHnd.get();
  VuGraphicsPipeline& vuPipeline = shader->requestPipeline(mat->m_materialSettings);

  vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.m_pipeline);
}
void
VuRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();
  VkBufferCopy    copyRegion {};
  copyRegion.srcOffset = 0;
  copyRegion.dstOffset = 0;
  copyRegion.size      = size;
  vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
  endSingleTimeCommands(commandBuffer);
}
void
VuRenderer::uploadToImage(const VuImage& vuImage, const byte* data, const VkDeviceSize size) {
  // todo
  auto res = m_stagingBuffer.setData(data, size);

  transitionImageLayout(vuImage.m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

  copyBufferToImage(
      m_stagingBuffer.m_buffer, vuImage.m_image, vuImage.m_lastCreateInfo.width, vuImage.m_lastCreateInfo.height);

  transitionImageLayout(
      vuImage.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}
void
VuRenderer::transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkImageMemoryBarrier barrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  barrier.oldLayout                       = oldLayout;
  barrier.newLayout                       = newLayout;
  barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
  barrier.image                           = image;
  barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel   = 0;
  barrier.subresourceRange.levelCount     = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount     = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage      = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage      = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  } else {
    throw std::invalid_argument("Unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, ZERO_FLAG, 0, nullptr, 0, nullptr, 1, &barrier);
  endSingleTimeCommands(commandBuffer);
}
void
VuRenderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
  VkCommandBuffer commandBuffer = beginSingleTimeCommands();

  VkBufferImageCopy region {};
  region.bufferOffset                    = 0;
  region.bufferRowLength                 = 0;
  region.bufferImageHeight               = 0;
  region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel       = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount     = 1;
  region.imageOffset                     = VkOffset3D {0, 0, 0};
  region.imageExtent                     = VkExtent3D {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  endSingleTimeCommands(commandBuffer);
}

void
VuRenderer::initImGui() {

  VkDescriptorPoolSize pool_sizes[] = {{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, .descriptorCount = 1000}};

  VkDescriptorPoolCreateInfo poolInfo {.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
  poolInfo.poolSizeCount = 1;
  poolInfo.pPoolSizes    = pool_sizes;
  poolInfo.maxSets       = 1000;
  poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

  VkResult descPoolRes =
      vkCreateDescriptorPool(m_vuDevice->m_device, &poolInfo, NO_ALLOC_CALLBACK, &m_uiDescriptorPool);
  THROW_if_fail(descPoolRes);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  //(void) io;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // io.ConfigFlags |= 1 << 7;
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForVulkan(m_window);

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance                  = m_vuDevice->m_vuPhysicalDevice->m_vuInstance->m_instance;
  init_info.PhysicalDevice            = m_vuDevice->m_vuPhysicalDevice->m_physicalDevice;
  init_info.Device                    = m_vuDevice->m_device;
  init_info.QueueFamily               = m_vuDevice->m_vuPhysicalDevice->m_indices.graphicsFamily;
  init_info.Queue                     = m_vuDevice->m_graphicsQueue;
  init_info.DescriptorPool            = m_uiDescriptorPool;
  init_info.MinImageCount             = 2;
  init_info.ImageCount                = 2;
  init_info.UseDynamicRendering       = false;
  init_info.RenderPass                = m_deferredRenderSpace.m_lightningPass->m_renderPass;

  ImGui_ImplVulkan_Init(&init_info);

  ImGui_ImplVulkan_CreateFontsTexture();


}

} // namespace Vu
