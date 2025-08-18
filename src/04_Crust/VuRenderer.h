#pragma once
#include "01_InnerCore/IndexAllocator.h"
#include "01_InnerCore/TypeDefs.h"
#include "02_OuterCore/VuConfig.h"
#include "03_Mantle/VuBuffer.h"
#include "03_Mantle/VuSurface.h"
#include "03_Mantle/VuTypes.h"
#include "SDL3/SDL.h"
#include "VuDeferredRenderSpace.h"

struct ImGui_ImplVulkanH_Window;
namespace vk {
enum class Format;
enum class ImageLayout;
} // namespace vk

namespace Vu {

struct VuDevice;
struct VuPhysicalDevice;
struct VuInstance;
struct VuMaterial;
struct VuMesh;
} // namespace Vu

namespace Vu {
struct VuSampler;

struct VuRendererCreateInfo {

  // bindings that correspond to global bindless desc set
  uint32_t uboBinding {0u};
  uint32_t samplerBinding {1u};
  uint32_t sampledImageBinding {2u};
  uint32_t storageImageBinding {3u};
  uint32_t storageBufferBinding {4u};

  uint32_t uboCount {1u};
  uint32_t samplerCount {256u};
  uint32_t sampledImageCount {256u};
  uint32_t storageImageCount {256u};
  uint32_t storageBufferCount {256u};
};
// #####################################################################################################################

struct VuRenderer {

  std::shared_ptr<VuInstance>       m_vuInstance {};
  std::shared_ptr<VuPhysicalDevice> m_vuPhysicalDevice {};
  SDL_Window*                       m_window {};
  SDL_Event                         m_sdlEvent {};
  std::shared_ptr<VuSurface>        m_vuSurface {};
  std::shared_ptr<VuDevice>         m_vuDevice {};
  VuDeferredRenderSpace             m_deferredRenderSpace {};
  ImGui_ImplVulkanH_Window*         m_imguiMainWindowData {};
  //
  VkCommandPool         m_commandPool {nullptr};
  VkDescriptorPool      m_descriptorPool {nullptr};
  VkDescriptorPool      m_uiDescriptorPool {nullptr};
  VkDescriptorSetLayout m_globalDescriptorSetLayout {nullptr};
  VkPipelineLayout      m_globalPipelineLayout {nullptr};
  //
  std::vector<VkDescriptorSet> m_globalDescriptorSets {};
  std::vector<VkCommandBuffer> m_commandBuffers {};
  //
  std::vector<VkSemaphore> m_imageAvailableSemaphores {};
  std::vector<VkSemaphore> m_renderFinishedSemaphores {};
  std::vector<VkFence>     m_inFlightFences {};
  //
  std::vector<VuBuffer> m_uniformBuffers {};
  u32                   m_currentFrame {};
  u32                   m_currentFrameImageIndex {};
  // VuDisposeStack               m_disposeStack {};
  IndexAllocator             m_imgBindlessIndexAllocator;
  IndexAllocator             m_samplerBindlessIndexAllocator;
  IndexAllocator             m_bufferBindlessIndexAllocator;
  IndexAllocator             m_materialDataBindlessIndexAllocator;
  GPU::FrameConstant              m_frameConstant {};
  float                      m_deltaAsSecond {};
  u64                        m_prevTimeAsNanoSecond {};
  float                      m_mouseX {};
  float                      m_mouseY {};
  float                      m_mouseDeltaX {};
  float                      m_mouseDeltaY {};
  std::shared_ptr<VuBuffer>  m_debugBuffer {};
  std::shared_ptr<VuBuffer>  m_materialDataBuffer {};
  std::shared_ptr<VuImage>   m_defaultImage {};
  std::shared_ptr<VuImage>   m_defaultNormalImage {};
  std::shared_ptr<VuSampler> m_defaultSampler {};

private:
  // holds the address of all other buffers
  VuBuffer             m_bdaBuffer {};
  VuBuffer             m_stagingBuffer {};
  VuRendererCreateInfo m_lastCreateInfo {};

public:
  explicit VuRenderer(const VuRendererCreateInfo& createInfo);

  VuRenderer(const VuRenderer& other) = delete;
  VuRenderer&
  operator=(const VuRenderer& other) = delete;

  VuRenderer(VuRenderer&& other) noexcept = default;
  VuRenderer&
  operator=(VuRenderer&& other) noexcept = default;

  ~VuRenderer();

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
  pushConstants(const GPU::PushConstant& pushConstant);

  void
  drawIndexed(u32 indexCount) const;

  void
  beginImgui() const;

  void
  endImgui() const;

  void
  updateFrameConstantBuffer(GPU::FrameConstant ubo) const;

  void
  preUpdate();

  void
  pollUserInput();

  static float
  time();

  void
  waitForFences() const;

  void
  resetSwapChain();

  void
  bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer) const;

  void
  initImGui();
  // ###################################################################################################################
  // ###################################################################################################################
  // ###################################################################################################################
  // ###################################################################################################################

  void
  writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const;

  void
  registerToBindless(VuBuffer& vuBuffer);

  void
  registerToBindless(VuImage& vuImage);

  void
  registerToBindless(VuSampler& vuSampler);

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

  [[nodiscard]] VkCommandBuffer
  beginSingleTimeCommands() const;

  void
  endSingleTimeCommands(const VkCommandBuffer& commandBuffer) const;

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  /// RESOURCES
  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void
  initBindlessResourceManager(const VuRendererCreateInfo& info);

  VuImage
  createImageFromAsset(const path& path, VkFormat format);

  std::shared_ptr<GPU::VuMaterialDataHandle>
  createMaterialDataIndex();

  static void
  bindMaterial(const VkCommandBuffer& cb, const std::shared_ptr<VuMaterial>& material);

  void
  copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

  void
  uploadToImage(const VuImage& vuImage, const byte* data, VkDeviceSize size);

  void
  transitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

  void
  copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

  std::span<std::byte, Vu::config::MATERIAL_DATA_SIZE>
  getMaterialDataSpan(const std::shared_ptr<GPU::VuMaterialDataHandle>& handle) const;

  template <typename T>
  T*
  getMaterialDataPointerAs(const GPU::VuMaterialDataHandle handle) {
    static_assert(sizeof(T) == config::MATERIAL_DATA_SIZE, "Material data type mismatch");
    const size_t offset  = config::MATERIAL_DATA_SIZE * handle.index;
    byte*        dataPtr = static_cast<byte*>(m_materialDataBuffer->m_mapPtr);
    return reinterpret_cast<T*>(dataPtr + offset);
  }
};
} // namespace Vu
