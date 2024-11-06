#pragma once

#include "Common.h"

#include "SDL3/SDL.h"

#include "volk.h"
#include "vk_mem_alloc.h"
#include "VuTypes.h"

namespace Vu {

struct VuRenderer;


    namespace ctx {
        constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        inline VuFrameConst frameConst;


        inline VmaAllocator vma;
        inline VkInstance instance;
        inline VkPhysicalDevice physicalDevice;
        inline VkDevice device;
        inline VkQueue graphicsQueue;
        inline VkQueue presentQueue;


        inline VkCommandPool commandPool;
        inline VkDescriptorPool descriptorPool;
        inline VkDescriptorPool uiDescriptorPool;

        //inline VkDescriptorSetLayout frameConstantsDescriptorSetLayout;
        //inline std::vector<VkDescriptorSet> frameConstantDescriptorSets;

        inline VkDescriptorSetLayout globalDescriptorSetLayout;
        inline std::vector<VkDescriptorSet> globalDescriptorSets;
        inline VkPipelineLayout globalPipelineLayout;


        inline VuRenderer* vuRenderer;
        inline SDL_Window* window;
        inline SDL_Event sdlEvent;

        inline float deltaAsSecond = 0;
        inline uint64 prevTimeAsNanoSecond = 0;
        inline float mouseX = 0;
        inline float mouseY = 0;

        inline float mouseDeltaX = 0;
        inline float mouseDeltaY = 0;




        inline void PreUpdate() {
            //nano => micro => mili => second
            deltaAsSecond = (SDL_GetTicksNS() - prevTimeAsNanoSecond) / 1000.0f / 1000.0f / 1000.0f;
            prevTimeAsNanoSecond = SDL_GetTicksNS();
        }

        inline void UpdateInput() {
            SDL_GetRelativeMouseState(&mouseDeltaX, &mouseDeltaY);
            SDL_GetMouseState(&mouseX, &mouseY);
        }

        inline float time() {
            return  SDL_GetTicks() / 1000.0f;
        }


        inline VkCommandBuffer BeginSingleTimeCommands() {
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = commandPool;
            allocInfo.commandBufferCount = 1;

            VkCommandBuffer commandBuffer;
            VkCheck(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

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

            vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
            vkQueueWaitIdle(graphicsQueue);

            vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
        }

    }
}
