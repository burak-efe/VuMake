#pragma once

#include "Common.h"
//#include "GLFW/glfw3.h"
#include "SDL3/SDL.h"


class VuRenderer;

namespace Vu {
    inline VmaAllocator VmaAllocator;
    inline VkInstance Instance;
    inline VkPhysicalDevice PhysicalDevice;
    inline VkDevice Device;

    inline VkCommandPool commandPool;
    inline VkQueue graphicsQueue;
    inline VkQueue presentQueue;

    inline VuRenderer* Renderer;
    //inline GLFWwindow* window;
    inline SDL_Window* sdlWindow;
    inline SDL_Event sdlEvent;

    inline float DeltaAsSecond = 0;
    inline uint64 PrevTimeAsNanoSecond = 0;
    inline float MouseX = 0;
    inline float MouseY = 0;

    inline float MouseDeltaX = 0;
    inline float MouseDeltaY = 0;


    inline VkCommandBuffer BeginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = Vu::commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(Vu::Device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    inline void EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(Vu::graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(Vu::graphicsQueue);

        vkFreeCommandBuffers(Vu::Device, Vu::commandPool, 1, &commandBuffer);
    }

    inline void PreUpdate() {
        //nano => micro => mili => second
        DeltaAsSecond = (SDL_GetTicksNS() - PrevTimeAsNanoSecond) / 1000.0f / 1000.0f / 1000.0f;
        PrevTimeAsNanoSecond = SDL_GetTicksNS();


    }

    inline void UpdateInput() {

        SDL_GetRelativeMouseState(&MouseDeltaX, &MouseDeltaY);
        //::cout << MouseDeltaX  << "   " << MouseDeltaY<< std::endl;
        SDL_GetMouseState(&MouseX, &MouseY);
    }


}
