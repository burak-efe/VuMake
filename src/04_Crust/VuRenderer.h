#pragma once

#include <array>                    // for array
#include <functional>               // for function
#include <memory>                   // for shared_ptr
#include <stack>                    // for stack

#include "../02_OuterCore/VuConfig.h"
#include "01_InnerCore/TypeDefs.h" // for u32
#include "12_VuMakeCore/VuTypes.h" // for GPU_FrameConst (ptr only), GPU_Pu...
#include "VuDevice.h"              // for VuDevice
#include "VuSwapChain.h"           // for VuSwapChain

struct ImGui_ImplVulkanH_Window;

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
    // vk::Instance               instance{};
    // vk::PhysicalDevice         physicalDevice{};

    VuDevice                   vuDevice;
    //vk::DebugUtilsMessengerEXT debugMessenger{};
    vk::SurfaceKHR             surface{};
    VuSwapChain                swapChain{};
    ImGui_ImplVulkanH_Window*  imguiMainWindowData{};
    VuDisposeStack             disposeStack{};
    vector<vk::CommandBuffer> commandBuffers{};
    vector<vk::Semaphore> imageAvailableSemaphores{};
    vector<vk::Semaphore> renderFinishedSemaphores{};
    vector<vk::Fence>     inFlightFences{};
    vector<VuBuffer> uniformBuffers{};
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
    void bindGlobalBindlessSet(const vk::CommandBuffer& commandBuffer);
    void initImGui();
};
}
