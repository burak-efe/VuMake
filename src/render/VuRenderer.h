#pragma once

#include <functional>
#include <stack>

#include "Common.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"
#include "VuMaterialData.h"

#include "Mesh.h"
#include "VuGraphicsPipeline.h"
#include "VuSwapChain.h"
#include "Vu.h"
#include "VuMaterial.h"
#include "VuSampler.h"
#include "VuTexture.h"
#include "SDL3/SDL_vulkan.h"

constexpr uint32 WIDTH = 1280;
constexpr uint32 HEIGHT = 720;

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

class VuRenderer {
public:
    VkDebugUtilsMessengerEXT debugMessenger;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    std::vector<VuBuffer> uniformBuffers;

    VkSurfaceKHR surface;
    Vu::VuSwapChain swapChain;
    //VuGraphicsPipeline debugPipeline;
    ImGui_ImplVulkanH_Window imguiMainWindowData;

    uint32 currentFrame = 0;
    uint32 currentFrameImageIndex = 0;

    VuTexture debugTexture;
    VuSampler debugSampler;

    VuMaterialDataPool materialDataPool;


    // Select a binding for each descriptor type
    const uint32 STORAGE_BINDING = 0;
    const uint32 SAMPLER_BINDING = 1;
    const uint32 IMAGE_BINDING = 2;
    // Max count of each descriptor type
    // You can query the max values for these with
    // physicalDevice.getProperties().limits.maxDescriptrorSet*******
    const uint32 STORAGE_COUNT = 256;
    const uint32 SAMPLER_COUNT = 125;
    const uint32 IMAGE_COUNT = 256;

    uint32 lastImageResource;
    uint32 lastSamplerResource;
    uint32 lastStorageResource;

    std::stack<std::function<void()>> disposeStack;


    void updateGlobalSets() {



        for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {

            for (size_t j = 0; j < VuTexture::allTextures.size(); j++) {

                VkDescriptorImageInfo imageInfo{
                    .sampler = VK_NULL_HANDLE,
                    .imageView = VuTexture::allTextures[j].textureImageView,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                };

                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = Vu::globalDescriptorSets[i];
                descriptorWrite.dstBinding = IMAGE_BINDING;
                descriptorWrite.dstArrayElement = j;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrite.descriptorCount = 1;
                //descriptorWrite.pBufferInfo = &bufferInfo;
                descriptorWrite.pImageInfo = &imageInfo;

                vkUpdateDescriptorSets(Vu::Device, 1, &descriptorWrite, 0, nullptr);
            }

            VkDescriptorImageInfo imageInfo{
                .sampler = debugSampler.vkSampler,
            };

            //sampler
            VkWriteDescriptorSet samplerWrite{};
            samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerWrite.dstSet = Vu::globalDescriptorSets[i];
            samplerWrite.dstBinding = SAMPLER_BINDING;
            samplerWrite.dstArrayElement = 0;
            samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
            samplerWrite.descriptorCount = 1;
            samplerWrite.pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(Vu::Device, 1, &samplerWrite, 0, nullptr);
        }

    }



void Init();

void Dispose();

bool ShouldWindowClose();

void WaitIdle();

void BeginFrame();

void EndFrame();

void BindMesh(const Mesh& mesh);

void BindMaterial(const VuMaterial& material, VuPushConstant pushConstant);

void DrawIndexed(uint32 indexCount);

void BeginImgui();

void EndImgui();

void UpdateUniformBuffer(FrameUBO ubo);

//void PushConstants(VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* pValues);

void BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

void EndRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

void InitWindow();

//void InitVulkan();

void InitVulkanDevice();

void CreateVulkanMemoryAllocator();

void CreateSurface();

void CreateSwapChain();

//void CreateGraphicsPipeline();

void CreateCommandPool();

void CreateCommandBuffers();

void CreateSyncObjects();

void ResetSwapChain();

void CreateDescriptorPool();

void CreateDescriptorSets();

void CreateDescriptorSetLayout();

void CreateUniformBuffers();

void SetupImGui();

//static void MouseCallback(GLFWwindow* window, double xpos, double ypos);

};
