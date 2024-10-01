#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS


#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_impl_glfw.h"
#include "GLFW/glfw3.h"




#include "Mesh.h"
#include "VuGraphicsPipeline.h"
#include "VuSwapChain.h"
#include "VuDepthStencil.h"
#include "Vu.h"

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
    //GLFWwindow* window;
    //inline static VkCommandPool commandPool;
    //inline static VkQueue graphicsQueue;
    //VkQueue presentQueue;
    float PrevTime = 0;
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
    uint32 currentFrameImageIndex = 0;
    VkDescriptorPool uiDescriptorPool;

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

    void RecreateSwapChain();

    void CreateDescriptorPool();

    void CreateDescriptorSets();

    void CreateDescriptorSetLayout();

    void CreateUniformBuffers();

    void SetupImGui();
    static void MouseCallback(GLFWwindow* window, double xpos, double ypos);




};
