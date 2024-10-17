#pragma once

#include "Common.h"
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_sdl3.h"

#include "Mesh.h"
#include "VuGraphicsPipeline.h"
#include "VuSwapChain.h"
#include "Vu.h"
#include "VuTexture.h"
#include "SDL3/SDL_vulkan.h"

constexpr int MAX_FRAMES_IN_FLIGHT = 2;
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

    VkDescriptorPool DescriptorPool;
    VkDescriptorPool UI_DescriptorPool;

    VkDescriptorSetLayout FrameConstantsDescriptorSetLayout;
    std::vector<VkDescriptorSet> FrameConstantDescriptorSets;

    VkDescriptorSetLayout ImageDescriptorSetLayout;
    std::vector<VkDescriptorSet> ImageDescriptorSets;

    VkSurfaceKHR Surface;
    Vu::VuSwapChain SwapChain;
    VuGraphicsPipeline DebugPipeline;
    ImGui_ImplVulkanH_Window g_MainWindowData;

    uint32 currentFrame = 0;
    uint32 currentFrameImageIndex = 0;

    VuTexture DebugTexture;


    void Init();

    void Dispose();

    bool ShouldWindowClose();

    void WaitIdle();

    void BeginFrame();

    void EndFrame();

    void RenderMesh(Mesh& mesh, glm::mat4 trs);

    void BeginImgui();

    void EndImgui();

    void UpdateUniformBuffer(FrameUBO ubo);

    void PushConstants(VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* pValues);

    void BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

    void EndRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

    void InitWindow();

    void InitVulkan();

    void CreateVulkanMemoryAllocator();

    void CreateSurface();

    void CreateSwapChain();

    void CreateGraphicsPipeline();

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

    VkDeviceSize aligned_size(VkDeviceSize value, VkDeviceSize alignment) {
        return (value + alignment - 1) & ~(alignment - 1);
    }

    VkDeviceAddress get_device_address(VkDevice device, VkBuffer buffer) {
        VkBufferDeviceAddressInfo deviceAdressInfo{};
        deviceAdressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        deviceAdressInfo.buffer = buffer;
        uint64_t address = vkGetBufferDeviceAddress(device, &deviceAdressInfo);
        return address;
    }
};
