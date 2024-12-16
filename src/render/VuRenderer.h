#pragma once

#include <functional>
#include <stack>

#include "SDL3/SDL_vulkan.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"

#include "Common.h"
#include "VuMesh.h"
#include "VuSwapChain.h"
#include "VuBuffer.h"
#include "VuMaterial.h"
#include "VuSampler.h"
#include "VuTexture.h"
#include "VuResourceManager.h"

namespace Vu {
    constexpr uint32 WIDTH = 1280;
    constexpr uint32 HEIGHT = 720;

    struct VuRenderer {
    public:
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

        //VuMaterialDataPool materialDataPool;

        std::stack<std::function<void()> > disposeStack;

        void init();

        void initWindow();

        void uninit();

        bool shouldWindowClose();

        void waitIdle();

        void beginFrame();

        void endFrame();

        void bindMesh(const VuMesh& mesh);

        void bindMaterial(const VuMaterial& material);

        void drawIndexed(uint32 indexCount);

        void pushConstants(const GPU_PushConstant& pushConstant);

        void beginImgui();

        void endImgui();

        void updateFrameConstantBuffer(GPU_FrameConst ubo);

        void reloadShaders();

    private:
        void waitForFences();

        void beginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void endRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);

        void initSDL();

        void initVulkanDevice();

        void initVulkanInstance();

        void initImGui();

        void initSurface();

        void initSwapchain();

        void initCommandBuffers();

        void initSyncObjects();

        void resetSwapChain();

        void initUniformBuffers();

        void bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer);

    };
}
