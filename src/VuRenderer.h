#pragma once

#include "imgui/imgui_impl_vulkan.h"

#include "Mesh.h"
#include "VuGraphicsPipeline.h"
#include "VuSwapChain.h"
#include "VuDepthStencil.h"

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
    GLFWwindow *window;
    std::vector<Mesh *> meshes;
    inline static VkCommandPool commandPool;
    inline static VkQueue graphicsQueue;
    VkQueue presentQueue;

    void Init();

    bool ShouldWindowClose();

    void Tick();

    void Cleanup();

    void WaitIdle();

private:
    VkDebugUtilsMessengerEXT debugMessenger;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void *> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    VkSurfaceKHR surface;
    Vu::VuSwapChain SwapChain;
    VuDepthStencil DepthStencil;

    VuGraphicsPipeline DebugPipeline;

    ImGui_ImplVulkanH_Window g_MainWindowData;

    uint32 currentFrame = 0;
    double PrevTime = 0;

    VkDescriptorPool uiDescriptorPool;


    void InitWindow();

    void InitVulkan();

    void CreateVulkanMemoryAllocator();

    void SetupDebugMessenger();

    void CreateSurface();

    void CreateSwapChain();

    void CreateGraphicsPipeline();

    void CreateCommandPool();

    void CreateCommandBuffers();



    void CreateSyncObjects();

    void UpdateFpsCounter();

    void RecreateSwapChain();

    void CreateDescriptorPool();

    void CreateDescriptorSets();

    void UpdateUniformBuffer(uint32 currentImage);

    void CreateDescriptorSetLayout();

    void CreateUniformBuffers();

    void SetupImGui();

    void DrawFrame();

    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex);

    void BeginRecord();
    void EndRecord();
};