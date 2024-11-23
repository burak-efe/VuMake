#pragma once

#include <functional>
#include <stack>

#include "Common.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"
#include "VuMaterialDataPool.h"

#include "VuMesh.h"
#include "VuSwapChain.h"
#include "VuBuffer.h"
//#include "VuGlobalSetManager.h"
#include "VuMaterial.h"
#include "VuSampler.h"
#include "VuTexture.h"
#include "VuConfig.h"
#include "VuHandle.h"
#include "SDL3/SDL_vulkan.h"

namespace Vu {

    constexpr uint32 WIDTH = 1280;
    constexpr uint32 HEIGHT = 720;



#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
    constexpr bool enableValidationLayers = true;
#endif


    struct VuRenderer {
    public:
        VkDebugUtilsMessengerEXT debugMessenger;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        std::vector<VuBuffer> uniformBuffers;

        VkSurfaceKHR surface;
        VuSwapChain swapChain;
        ImGui_ImplVulkanH_Window imguiMainWindowData;

        uint32 currentFrame = 0;
        uint32 currentFrameImageIndex = 0;

        VuTextureHandle debugTexture;
        VuSampler debugSampler;

        VuMaterialDataPool materialDataPool;


        uint32 lastImageResource;
        uint32 lastSamplerResource;
        uint32 lastStorageResource;

        std::stack<std::function<void()> > disposeStack;

        //VuGlobalSetManager globalSetManager;


        // void WriteTexture(uint32 writeIndex, VuTexture& texture);
        //
        // void WriteSampler(uint32 writeIndex, VkSampler& sampler);


        void Init();

        void Dispose();

        bool ShouldWindowClose();

        void WaitIdle();

        void BeginFrame();

        void EndFrame();

        void BindMesh(const VuMesh& mesh);

        void BindMaterial(const VuMaterial& material);

        void DrawIndexed(uint32 indexCount);

        void pushConstants(const VuPushConstant& pushConstant) {
            auto commandBuffer = commandBuffers[currentFrame];
            vkCmdPushConstants(commandBuffer, ctx::globalPipelineLayout, VK_SHADER_STAGE_ALL, 0, sizeof(VuPushConstant), &pushConstant);
        }

        void BeginImgui();

        void EndImgui();

        void UpdateUniformBuffer(VuFrameConst ubo);

        void BeginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void EndRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void InitWindow();

        void InitVulkanDevice();

        void CreateVulkanMemoryAllocator();

        void CreateSurface();

        void CreateSwapChain();

        void CreateCommandPool();

        void CreateCommandBuffers();

        void CreateSyncObjects();

        void ResetSwapChain();

        void CreateDescriptorPool();

        void CreateDescriptorSets();

        void CreateDescriptorSetLayout();

        void CreateUniformBuffers();

        void SetupImGui();

        void reloadShaders();


        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                    vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

            if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
            }
        }

        void bindGlobalSet(const VkCommandBuffer& commandBuffer) {
            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                ctx::globalPipelineLayout,
                0,
                1,
                &ctx::globalDescriptorSets[currentFrame],
                0,
                nullptr
            );
        }
    };
}
