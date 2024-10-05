#pragma once

#include <vector>
#include "Common.h"
#include "VuUtils.h"
#include "GLFW/glfw3.h"


//class GLFWwindow;

namespace Vu {
    class VuSwapChain {
    private:
        void CreateImageViews(VkDevice device);

    public:
        VkFormat colorFormat;
        VkColorSpaceKHR colorSpace;
        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        uint32_t imageCount;
        uint32_t queueNodeIndex = UINT32_MAX;
        VkFormat swapChainImageFormat;
        VkExtent2D swapChainExtent;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        void CreateSwapChain(GLFWwindow* window, VkSurfaceKHR surface);

        void CleanupSwapchain();

        static SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);

        static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

        static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

        static VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);

        static QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);
    };
}
