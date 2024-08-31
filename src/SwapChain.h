#pragma once

#include <vector>
#include<vulkan/vulkan.h>

#include "VulkanTypes.h"
#include "GLFW/glfw3.h"

// typedef struct SwapChainBuffers {
//     VkImage image;
//     VkImageView view;
// } SwapChainBuffer;
namespace VuMake {
    class SwapChain {
    private:
        VkInstance _instance;
        VkDevice _device;
        VkPhysicalDevice _physicalDevice;

        //  VkSurfaceKHR surface;
        void CreateImageViews(VkDevice device);

    public:
        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        uint32_t imageCount;
        std::vector<VkImage> swapChainImages;
        //std::vector<SwapChainBuffer> buffers;
        uint32_t queueNodeIndex = UINT32_MAX;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkFramebuffer> swapChainFramebuffers;
        std::vector<VkImageView> swapChainImageViews;


        void SetContext(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);

        void CreateSwapChain(GLFWwindow *window, VkSurfaceKHR surface);

        void CleanupSwapchain();

        VkResult acquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t *imageIndex);

        VkResult queuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);


        static SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);

        static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities, GLFWwindow *window);

        static QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    };
}
