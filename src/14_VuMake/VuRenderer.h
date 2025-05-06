#pragma once

#include <array>                    // for array
#include <functional>               // for function
#include <memory>                   // for shared_ptr
#include <stack>                    // for stack

#include <vulkan/vulkan_core.h>     // for VkCommandBuffer, VkSemaphore, VkD...
#include "imgui_impl_vulkan.h"      // for ImGui_ImplVulkanH_Window

#include "08_LangUtils/TypeDefs.h"  // for u32
#include "11_Config/VuConfig.h"     // for MAX_FRAMES_IN_FLIGHT
#include "12_VuMakeCore/VuTypes.h"  // for GPU_FrameConst (ptr only), GPU_Pu...
#include "VuDevice.h"               // for VuDevice
#include "VuSwapChain.h"            // for VuSwapChain

namespace Vu
{
struct VuBuffer;
struct VuMaterial;
struct VuMesh;
}

namespace Vu
{
struct VuRenderer
{
    VkInstance               instance{};
    VkPhysicalDevice         physicalDevice{};
    VuDevice                 vuDevice{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    VkSurfaceKHR             surface{};
    VuSwapChain              swapChain{};
    ImGui_ImplVulkanH_Window imguiMainWindowData{};
    VuDisposeStack           disposeStack{};

    vector<VkCommandBuffer> commandBuffers{};

    vector<VkSemaphore>     imageAvailableSemaphores{};
    vector<VkSemaphore>     renderFinishedSemaphores{};
    vector<VkFence>         inFlightFences{};

    vector<VuBuffer>        uniformBuffers{};

    u32 currentFrame{};
    u32 currentFrameImageIndex{};

    VuRenderer();
    void uninit();
    bool shouldWindowClose();
    void waitIdle();

    void beginFrame();
    void beginLightningPass();
    void endFrame();

    void bindMesh(VuMesh& mesh);
    void bindMaterial(std::shared_ptr<VuMaterial>& material);
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
