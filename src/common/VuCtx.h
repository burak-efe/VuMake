#pragma once

#include "Common.h"

#include "SDL3/SDL.h"
#include "volk.h"

//#include "VuDevice.h"
#include "VuTypes.h"


namespace Vu {

    struct VuDevice;
    struct VuRenderer;

    namespace ctx {

        inline VuDevice* vuDevice{};
        inline GPU_FrameConst frameConst{};
        inline VuRenderer* vuRenderer = nullptr;
        inline SDL_Window* window = nullptr;
        inline SDL_Event sdlEvent{};

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
            return SDL_GetTicks() / 1000.0f;
        }




    }
}
