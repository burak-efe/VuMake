#pragma once

#include "Common.h"
#include "SDL3/SDL.h"

#include "volk.h"
#include "vk_mem_alloc.h"

class VuRenderer;

namespace Vu {
    constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    inline VmaAllocator VmaAllocator;
    inline VkInstance Instance;
    inline VkPhysicalDevice PhysicalDevice;
    inline VkDevice Device;

    inline VkCommandPool commandPool;
    inline VkDescriptorPool DescriptorPool;
    inline VkDescriptorPool UI_DescriptorPool;

    inline VkDescriptorSetLayout FrameConstantsDescriptorSetLayout;
    inline std::vector<VkDescriptorSet> frameConstantDescriptorSets;

    inline VkDescriptorSetLayout globalDescriptorSetLayout;
    inline std::vector<VkDescriptorSet> globalDescriptorSets;
    inline VkPipelineLayout globalPipelineLayout;

    inline VkQueue graphicsQueue;
    inline VkQueue presentQueue;

    inline VuRenderer* Renderer;
    inline SDL_Window* Window;
    inline SDL_Event sdlEvent;

    inline float DeltaAsSecond = 0;
    inline uint64 PrevTimeAsNanoSecond = 0;
    inline float MouseX = 0;
    inline float MouseY = 0;

    inline float MouseDeltaX = 0;
    inline float MouseDeltaY = 0;

    inline void PreUpdate() {
        //nano => micro => mili => second
        DeltaAsSecond = (SDL_GetTicksNS() - PrevTimeAsNanoSecond) / 1000.0f / 1000.0f / 1000.0f;
        PrevTimeAsNanoSecond = SDL_GetTicksNS();
    }

    inline void UpdateInput() {
        SDL_GetRelativeMouseState(&MouseDeltaX, &MouseDeltaY);
        SDL_GetMouseState(&MouseX, &MouseY);
    }

}
