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
#include "VuMaterial.h"
#include "VuSampler.h"
#include "VuTexture.h"
#include "SDL3/SDL_vulkan.h"

namespace Vu {

    constexpr uint32 WIDTH = 1280;
    constexpr uint32 HEIGHT = 720;

    constexpr uint32 UBO_BINDING = 0;
    constexpr uint32 STORAGE_BINDING = 1;
    constexpr uint32 SAMPLER_BINDING = 2;
    constexpr uint32 IMAGE_BINDING = 3;

    constexpr uint32 UNIFORM_COUNT = 2;
    constexpr uint32 STORAGE_COUNT = 256;
    constexpr uint32 SAMPLER_COUNT = 125;
    constexpr uint32 IMAGE_COUNT = 256;

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

        VuTexture debugTexture;
        VuSampler debugSampler;

        VuMaterialDataPool materialDataPool;


        uint32 lastImageResource;
        uint32 lastSamplerResource;
        uint32 lastStorageResource;

        std::stack<std::function<void()> > disposeStack;




        void WriteTexture(uint32 writeIndex, VuTexture& texture);

        void WriteSampler(uint32 writeIndex, VkSampler& sampler);


        void Init();

        void Dispose();

        bool ShouldWindowClose();

        void WaitIdle();

        void BeginFrame();

        void EndFrame();

        void BindMesh(const VuMesh& mesh);

        void BindMaterial(const VuMaterial& material, VuPushConstant pushConstant);

        void DrawIndexed(uint32 indexCount);

        void BeginImgui();

        void EndImgui();

        void UpdateUniformBuffer(VuFrameConst ubo);

        void BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

        void EndRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

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


        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                  const VkAllocationCallbacks* pAllocator) {
            auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
                    vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

            if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
            }
        }
    };
}
