#pragma once

#include <functional>
#include <stack>

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"

#include "10_Core/VuCommon.h"
#include "VuMesh.h"
#include "VuSwapChain.h"
#include "VuBuffer.h"
#include "VuMaterial.h"
#include "VuDevice.h"


namespace Vu
{
    constexpr uint32 WIDTH  = 1280;
    constexpr uint32 HEIGHT = 720;

    struct VuRenderer
    {
        VuDevice vuDevice{};
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore>     imageAvailableSemaphores;
        std::vector<VkSemaphore>     renderFinishedSemaphores;
        std::vector<VkFence>         inFlightFences;
        std::vector<VuBuffer>        uniformBuffers;

        VkSurfaceKHR             surface;
        VuSwapChain              swapChain;
        ImGui_ImplVulkanH_Window imguiMainWindowData;

        uint32 currentFrame           = 0;
        uint32 currentFrameImageIndex = 0;

        VuHandle2<VuImage> defaultImageHandle;
        VuHandle2<VuSampler> defaultSamplerHandle;

        std::stack<std::function<void()>> disposeStack;

        void init();
        void uninit();
        bool shouldWindowClose();
        void waitIdle();
        void beginFrame();
        void endFrame();
        void bindMesh(VuMesh& mesh);
        void bindMaterial(const VuMaterial& material);
        void pushConstants(const GPU_PushConstant& pushConstant);
        void drawIndexed(uint32 indexCount);
        void beginImgui();
        void endImgui();
        void updateFrameConstantBuffer(GPU_FrameConst ubo);
        void reloadShaders();

    private:
        void waitForFences();
        void beginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);
        void endRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex);
        void resetSwapChain();
        void bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer);
        void initImGui();
    };
}
