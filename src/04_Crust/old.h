// #pragma once
//
// #include <cstdint>                        // for uint32_t
// #include <functional>                     // for function
// #include <memory>                         // for shared_ptr
// #include <optional>                       // for optional
// #include <span>                           // for span
// #include <stack>                          // for stack
// #include <vector>                         // for vector
//
// #include "01_InnerCore/IndexAllocator.h" // for IndexAllocator
// #include "01_InnerCore/TypeDefs.h"       // for u32, byte
// #include "02_OuterCore/Common.h"         // for path
// #include "12_VuMakeCore/VuBuffer.h"      // for VuBuffer, VuBufferCreateInf...
// #include "12_VuMakeCore/VuCommon.h"
// #include "12_VuMakeCore/VuImage.h"   // for VuImage, VuImageCreateInfo ...
// #include "12_VuMakeCore/VuSampler.h" // for VuSampler, VuSamplerCreateI...
// #include "12_VuMakeCore/VuTypes.h"   // for QueueFamilyIndices, VuDispo...
// #include "14_VuMake/VuMaterial.h"    // for MaterialSettings (ptr only)
// #include "VuShader.h"                // for VuShader
//
// namespace Vu
// {
// struct VuRenderPass;
// }
//
// namespace Vu
// {
// struct VuDeviceCreateInfo
// {
//
//     vk::Bool32                  enableValidationLayers;
//     vk::PhysicalDeviceFeatures2 requestedFeatures;
//     vk::SurfaceKHR              surface;
//     std::span<const char*>      deviceExtensions;
//
//     //bindless
//     u32 uboBinding;
//     u32 samplerBinding;
//     u32 sampledImageBinding;
//     u32 storageImageBinding;
//     u32 storageBufferBinding;
//
//     u32 uboCount;
//     u32 samplerCount;
//     u32 sampledImageCount;
//     u32 storageImageCount;
//     u32 storageBufferCount;
// };
//
// struct VuDevice
// {
//     vk::raii::Context                  raiiContext                        = {};
//     vk::raii::Instance                 instance                           = {nullptr};
//     vk::raii::PhysicalDevice           physicalDevice                     = {nullptr};
//     vk::raii::Device                   device                             = {nullptr};
//     vk::raii::CommandPool              commandPool                        = {nullptr};
//     vk::raii::DescriptorPool           descriptorPool                     = {nullptr};
//     vk::raii::DescriptorPool           uiDescriptorPool                   = {nullptr};
//     vk::raii::DescriptorSetLayout      globalDescriptorSetLayout          = {nullptr};
//     vk::raii::PipelineLayout           globalPipelineLayout               = {nullptr};
//     vk::DebugUtilsMessengerEXT         debugMessenger                     = {};
//     vk::PhysicalDeviceMemoryProperties memProperties                      = {};
//     VuQueueFamilyIndices                 queueFamilyIndices                 = {};
//     vk::Queue                          graphicsQueue                      = {};
//     vk::Queue                          presentQueue                       = {};
//     std::vector<vk::DescriptorSet>     globalDescriptorSets               = {};
//     IndexAllocator                     imgBindlessIndexAllocator          = {};
//     IndexAllocator                     samplerBindlessIndexAllocator      = {};
//     IndexAllocator                     bufferBindlessIndexAllocator       = {};
//     IndexAllocator                     materialDataBindlessIndexAllocator = {};
//
// private:
//     //holds the address of all other buffers
//     VuBuffer                   bdaBuffer                = {};
//     VuBuffer                   stagingBuffer            = {};
//     std::shared_ptr<VuBuffer>  debugBufferHnd           = {};
//     std::shared_ptr<VuBuffer>  materialDataBufferHandle = {};
//     std::shared_ptr<VuImage>   defaultImageHandle       = {};
//     std::shared_ptr<VuImage>   defaultNormalImageHandle = {};
//     std::shared_ptr<VuSampler> defaultSamplerHandle     = {};
//     VuDisposeStack             disposeStack             = {};
//
//     explicit VuDevice(const VuDeviceCreateInfo& createInfo);
//     //VuDevice() = default;
//
// public:
//     static std::expected<VuDevice, vk::Result> make(const VuDeviceCreateInfo& createInfo);
//
//     std::shared_ptr<VuImage>   createImage(const VuImageCreateInfo& info);
//     std::shared_ptr<VuImage>   createImageFromAsset(const path& path, vk::Format format);
//     std::shared_ptr<VuSampler> createSampler(const VuSamplerCreateInfo& info);
//     std::shared_ptr<VuBuffer>  createBuffer(const VuBufferCreateInfo& info);
//     std::shared_ptr<VuShader>  createShader(path vertexPath, path fragPath, VuRenderPass* vuRenderPass);
//
//     std::shared_ptr<u32> createMaterialDataIndex();
//
//     std::shared_ptr<VuMaterial> createMaterial(MaterialSettings     matSettings, std::shared_ptr<VuShader> shaderHnd,
//                                                std::shared_ptr<u32> materialDataHnd);
//
//     std::span<byte, 64> getMaterialData(std::shared_ptr<u32> handle);
//
//     void bindMaterial(vk::CommandBuffer cb, std::shared_ptr<VuMaterial> material);
//
//     void uninit();
//
//     void registerBindlessBDA_Buffer(const VuBuffer& buffer) const;
//
//     void writeUBO_ToGlobalPool(const VuBuffer& buffer, u32 writeIndex, u32 setIndex) const;
//
//     void registerToBindless(const VuBuffer& buffer, u32 bindlessIndex) const;
//
//     void registerToBindless(const vk::ImageView& imageView, u32 bindlessIndex) const;
//
//     void registerToBindless(const vk::Sampler& sampler, u32 bindlessIndex) const;
//
//     vk::CommandBuffer BeginSingleTimeCommands();
//
//     void EndSingleTimeCommands(vk::CommandBuffer commandBuffer);
//
//     void copyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
//
//     void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);
//
//     void uploadToImage(const VuImage& vuImage, const byte* data, const vk::DeviceSize size);
//
//     void transitionImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
//
//     void waitIdle();
//
// private:
//     void initDevice(const VuDeviceCreateInfo& info);
//
//     void initCommandPool(const VuDeviceCreateInfo& info);
//
//     void initBindlessDescriptorSetLayout(const VuDeviceCreateInfo& info);
//
//     void initDescriptorPool(const VuDeviceCreateInfo& info);
//
//     void initBindlessDescriptorSet();
//
//     void initPipelineLayout();
//
//     void initBindlessResourceManager(const VuDeviceCreateInfo& info);
//
//     void initDefaultResources();
// };
// }
