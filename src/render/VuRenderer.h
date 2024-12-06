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
#include "VuConfig.h"
#include "VuHandle.h"
#include "SDL3/SDL_vulkan.h"


namespace Vu {

    constexpr uint32 WIDTH = 1280;
    constexpr uint32 HEIGHT = 720;




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

        VuHandle<VuTexture> debugTexture;
        VuHandle<VuSampler> debugSampler;

        VuMaterialDataPool materialDataPool;

        std::stack<std::function<void()> > disposeStack;

        QueueFamilyIndices queueFamilies;
        VkPhysicalDeviceLimits deviceLimits;

        void init();

        void initWindow();

        void uninit();

        bool shouldWindowClose();

        void waitIdle();

        void beginFrame();

        void waitForFences();

        void endFrame();

        void BindMesh(const VuMesh& mesh);

        void BindMaterial(const VuMaterial& material);

        void DrawIndexed(uint32 indexCount);

        void pushConstants(const VuPushConstant& pushConstant);

        void BeginImgui();

        void EndImgui();

        void UpdateUniformBuffer(VuFrameConst ubo);

        void BeginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void EndRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void initSDL();

        void initVulkanDevice();

        void initVulkanPhysicalDevice();

        void initVulkanInstance();

        void SetupImGui();

        void initVMA();

        void initSurface();

        void initSwapchain();

        void initCommandPool();

        void CreateCommandBuffers();

        void CreateSyncObjects();

        void ResetSwapChain();

        void CreateDescriptorPool();

        void CreateDescriptorSets();

        void CreateDescriptorSetLayout();

        void initUniformBuffers();

        void reloadShaders();

        void bindGlobalSet(const VkCommandBuffer& commandBuffer);

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
