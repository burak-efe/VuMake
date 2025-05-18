#pragma once

#include <memory> // for shared_ptr

#include "01_InnerCore/IndexAllocator.h"
#include "01_InnerCore/TypeDefs.h"
#include "02_OuterCore/VuConfig.h"
#include "03_Mantle/VuBuffer.h"
#include "03_Mantle/VuDeferredRenderSpace.h"
#include "03_Mantle/VuSampler.h"
#include "03_Mantle/VuTypes.h"
#include "SDL3/SDL.h"
#include "vulkan/vulkan_raii.hpp"
#include "VuShader.h"

struct ImGui_ImplVulkanH_Window;

namespace Vu {
struct VuDevice;
struct VuPhysicalDevice;
struct VuInstance;
struct VuMaterial;
struct VuMesh;
} // namespace Vu

namespace Vu {

struct VuRendererCreateInfo {

  // bindless
  u32 uboBinding           = {0u};
  u32 samplerBinding       = {1u};
  u32 sampledImageBinding  = {2u};
  u32 storageImageBinding  = {3u};
  u32 storageBufferBinding = {4u};

  u32 uboCount           = {1u};
  u32 samplerCount       = {256u};
  u32 sampledImageCount  = {256u};
  u32 storageImageCount  = {256u};
  u32 storageBufferCount = {256u};
};

struct VuRenderer {
  std::shared_ptr<VuInstance>           vuInstance          = {};
  SDL_Window*                           window              = {};
  SDL_Event                             sdlEvent            = {};
  std::shared_ptr<vk::raii::SurfaceKHR> surface             = {};
  std::shared_ptr<VuPhysicalDevice>     vuPhysicalDevice    = {};
  std::shared_ptr<VuDevice>             vuDevice            = {};
  VuDeferredRenderSpace                 deferredRenderSpace = {};
  ImGui_ImplVulkanH_Window*             imguiMainWindowData = {};
  //
  vk::raii::CommandPool         commandPool                 = {nullptr};
  vk::raii::DescriptorPool      descriptorPool              = {nullptr};
  vk::raii::DescriptorPool      uiDescriptorPool            = {nullptr};
  vk::raii::DescriptorSetLayout globalDescriptorSetLayout   = {nullptr};
  vk::raii::PipelineLayout      globalPipelineLayout        = {nullptr};
  vector<vk::DescriptorSet>     globalDescriptorSets        = {};
  //
  vector<vk::raii::CommandBuffer> commandBuffers            = {};
  vector<vk::raii::Semaphore>     imageAvailableSemaphores  = {};
  vector<vk::raii::Semaphore>     renderFinishedSemaphores  = {};
  vector<vk::raii::Fence>         inFlightFences            = {};
  //
  vector<VuBuffer> uniformBuffers                           = {};
  u32              currentFrame                             = {};
  u32              currentFrameImageIndex                   = {};
  //
  VuDisposeStack disposeStack                               = {};
  //
  IndexAllocator imgBindlessIndexAllocator                  = {};
  IndexAllocator samplerBindlessIndexAllocator              = {};
  IndexAllocator bufferBindlessIndexAllocator               = {};
  IndexAllocator materialDataBindlessIndexAllocator         = {};
  //
  GPU_FrameConst frameConst                                 = {};
  float          deltaAsSecond                              = {};
  u64            prevTimeAsNanoSecond                       = {};
  float          mouseX                                     = {};
  float          mouseY                                     = {};
  float          mouseDeltaX                                = {};
  float          mouseDeltaY                                = {};

  std::shared_ptr<VuBuffer>  debugBuffer           = {};
  std::shared_ptr<VuBuffer>  materialDataBuffer = {};
  std::shared_ptr<VuImage>   defaultImage       = {};
  std::shared_ptr<VuImage>   defaultNormalImage = {};
  std::shared_ptr<VuSampler> defaultSampler     = {};
private:
  // holds the address of all other buffers
  VuBuffer                   bdaBuffer                = {};
  VuBuffer                   stagingBuffer            = {};
  VuRendererCreateInfo       lastCreateInfo           = {};

public:
  explicit VuRenderer(const VuRendererCreateInfo& createInfo);

  [[nodiscard]] bool
  shouldWindowClose() const;

  void
  beginFrame();

  void
  beginLightningPass() const;

  void
  endFrame();

  void
  bindMesh(VuMesh& mesh);

  void
  bindMaterial(std::shared_ptr<VuMaterial>& material);

  void
  pushConstants(const GPU_PushConstant& pushConstant);

  void
  drawIndexed(u32 indexCount) const;

  void
  beginImgui() const;

  void
  endImgui();

  void
  updateFrameConstantBuffer(GPU_FrameConst ubo) const;

  void
  PreUpdate();

  void
  UpdateInput();

  static float
  time();

  void
  waitForFences() const;

  void
  resetSwapChain();

  void
  bindGlobalBindlessSet(const vk::CommandBuffer& commandBuffer) const;

  //   void
  //   initImGui();
  // ###########################################################################################################################################################################################
  // ###########################################################################################################################################################################################
  // ###########################################################################################################################################################################################
  // ###########################################################################################################################################################################################

  // void
  // registerBindlessBDA_Buffer(const VuBuffer& buffer) const {
  //   vk::DescriptorBufferInfo bufferInfo {};
  //   bufferInfo.buffer = buffer.buffer;
  //   bufferInfo.range  = buffer.sizeInBytes;
  //
  //   for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
  //     vk::WriteDescriptorSet descriptorWrite {};
  //     descriptorWrite.dstSet          = globalDescriptorSets[i];
  //     descriptorWrite.dstBinding      = config::BINDLESS_STORAGE_BUFFER_BINDING;
  //     descriptorWrite.dstArrayElement = 0;
  //     descriptorWrite.descriptorType  = vk::DescriptorType::eStorageBuffer;
  //     descriptorWrite.descriptorCount = 1;
  //     descriptorWrite.pBufferInfo     = &bufferInfo;
  //
  //     vuDevice->device.updateDescriptorSets(descriptorWrite, {});
  //   }
  // }

  void
  writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const;

  void
  registerToBindless(VuBuffer& vuBuffer) ;

  void
  registerToBindless(VuImage& vuImage) ;

  void
  registerToBindless(VuSampler& vuSampler) ;

  void
  uninit();

  void
  initCommandPool(const VuRendererCreateInfo& info);

  void
  initPipelineLayout();

  void
  initBindlessDescriptorSetLayout(const VuRendererCreateInfo& info);

  void
  initDefaultResources();

  void
  initDescriptorPool(const VuRendererCreateInfo& info);

  void
  initBindlessDescriptorSet();

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  [[nodiscard]] vk::raii::CommandBuffer
  BeginSingleTimeCommands() const;

  void
  EndSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// RESOURCE CREATE
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void
  initBindlessResourceManager(const VuRendererCreateInfo& info);

  // std::shared_ptr<Vu::VuImage>
  // createImage(const VuImageCreateInfo& info) {
  //   std::shared_ptr<VuImage> handle   = std::make_shared<VuImage>();
  //   VuImage*                 resource = handle.get();
  //   resource->init(device, memProperties, info);
  //   u32 bindlessIndex = imgBindlessIndexAllocator.allocate();
  //   registerToBindless(resource->imageView, bindlessIndex);
  //   resource->bindlessIndex = bindlessIndex;
  //   return handle;
  // }

  std::shared_ptr<Vu::VuImage>
  createImageFromAsset(const path& path, vk::Format format);

  // std::shared_ptr<Vu::VuSampler>
  // createSampler(const VuSamplerCreateInfo& info) {
  //   std::shared_ptr<VuSampler> handle   = std::make_shared<VuSampler>();
  //   VuSampler*                 resource = handle.get();
  //
  //   resource->init(device, info);
  //   uint32_t bindlessIndex = samplerBindlessIndexAllocator.allocate();
  //   registerToBindless(resource->sampler, bindlessIndex);
  //   resource->bindlessIndex = bindlessIndex;
  //   return handle;
  // }
  //
  // std::shared_ptr<Vu::VuBuffer>
  // createBuffer(const VuBufferCreateInfo& info) {
  //   std::shared_ptr<VuBuffer> handle   = std::make_shared<VuBuffer>();
  //   VuBuffer*                 resource = handle.get();
  //
  //   resource->init(device, vma, info);
  //   u32 bindlessIndex = bufferBindlessIndexAllocator.allocate();
  //   registerToBindless(*resource, bindlessIndex);
  //   resource->bindlessIndex = bindlessIndex;
  //   return handle;
  // }
  //
  // std::shared_ptr<Vu::VuShader>
  // createShader(path vertexPath, path fragPath, VuRenderPass* vuRenderPass) {
  //   std::shared_ptr<VuShader> handle   = std::make_shared<VuShader>();
  //   VuShader*                 resource = handle.get();
  //   resource->init(this, vertexPath, fragPath, vuRenderPass);
  //   return handle;
  // }

  std::shared_ptr<VuMaterialDataHandle>
  createMaterialDataIndex();

  // std::shared_ptr<Vu::VuMaterial>
  // createMaterial(MaterialSettings          matSettings,
  //                std::shared_ptr<VuShader> shaderHnd,
  //                std::shared_ptr<u32>      materialDataHnd) {
  //   std::shared_ptr<VuMaterial> handle   = std::make_shared<VuMaterial>();
  //   VuMaterial*                 resource = handle.get();
  //   assert(resource != nullptr);
  //   *resource = VuMaterial {this, matSettings, shaderHnd, materialDataHnd};
  //   return handle;
  // }

  std::span<std::byte, Vu::config::MATERIAL_DATA_SIZE>
  getMaterialDataSpan(const std::shared_ptr<VuMaterialDataHandle>& handle) const;

  static void
  bindMaterial(const vk::CommandBuffer& cb, const std::shared_ptr<VuMaterial>& material);

  void
  copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

  void
  uploadToImage(const VuImage& vuImage, const byte* data, const vk::DeviceSize size);

  void
  transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

  void
  copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
};
} // namespace Vu
