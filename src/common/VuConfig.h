#pragma once
#include "Common.h"

namespace Vu::config {
    constexpr uint32 MAX_FRAMES_IN_FLIGHT = 2;

    constexpr uint32 UBO_BINDING = 0;
    constexpr uint32 STORAGE_BINDING = 1;
    constexpr uint32 SAMPLER_BINDING = 2;
    constexpr uint32 IMAGE_BINDING = 3;

    constexpr uint32 UNIFORM_COUNT = 1;
    constexpr uint32 STORAGE_COUNT = 1;
    constexpr uint32 SAMPLER_COUNT = 16;
    constexpr uint32 IMAGE_COUNT = 256;
}





