#pragma once

#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"

#include "10_Core/VuCommon.h"

#include "VuMesh.h"
#include "VuSwapChain.h"
#include "VuMaterial.h"
#include "VuDevice.h"
#include "11_Config/VuConfig.h"


namespace Vu
{
    struct VuRenderer
    {
        VkInstance               instance;
        VkPhysicalDevice         physicalDevice;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkSurfaceKHR             surface;
        VuSwapChain              swapChain;
        VuDevice                 vuDevice;
        VuDisposeStack           disposeStack;
        ImGui_ImplVulkanH_Window imguiMainWindowData;


        std::array<VkCommandBuffer, config::MAX_FRAMES_IN_FLIGHT> commandBuffers;
        std::array<VkSemaphore, config::MAX_FRAMES_IN_FLIGHT>     imageAvailableSemaphores;
        std::array<VkSemaphore, config::MAX_FRAMES_IN_FLIGHT>     renderFinishedSemaphores;
        std::array<VkFence, config::MAX_FRAMES_IN_FLIGHT>         inFlightFences;
        std::array<VuBuffer, config::MAX_FRAMES_IN_FLIGHT>        uniformBuffers;


        u32 currentFrame           = 0;
        u32 currentFrameImageIndex = 0;


        void init();
        void uninit();
        bool shouldWindowClose();
        void waitIdle();

        void beginFrame();
        void beginLightningPass();
        void endFrame();

        void bindMesh(VuMesh& mesh);
        void bindMaterial(VuHnd<VuMaterial> material);
        void pushConstants(const GPU_PushConstant& pushConstant);
        void drawIndexed(u32 indexCount);
        void beginImgui();
        void endImgui();
        void updateFrameConstantBuffer(GPU_FrameConst ubo);

    private:
        void waitForFences();
        void resetSwapChain();
        void bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer);
        void initImGui();
    };
}
