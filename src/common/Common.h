#pragma once

#include <cstdint>

#include "volk.h"
#include "tracy/Tracy.hpp"

#include "VuFloat2.h"
#include "VuFloat3.h"
#include "VuFloat4.h"
#include "VuFloat4x4.h"
#include "VuQuaternion.h"

namespace Vu
{
    using uint8  = uint8_t;
    using uint16 = uint16_t;
    using uint32 = uint32_t;
    using uint64 = uint64_t;

    using int8  = int8_t;
    using int16 = int16_t;
    using int32 = int32_t;

    using float2     = Math::Float2;
    using float3     = Math::Float3;
    using float4     = Math::Float4;
    using float4x4   = Math::Float4x4;
    using quaternion = Math::Quaternion;

    using path   = std::filesystem::path;
    using string = std::string;
    //using span = std::span;

    __declspec(noinline) void VkCheck(VkResult res);

    template <typename T_From, typename T_To>
    std::span<T_To> rpCastSpan(std::span<T_From> source)
    {
        static_assert(sizeof(T_From) == sizeof(T_To), "T_From and T_To must be the same size for reinterpret casting.");
        return std::span<T_To>(
                               reinterpret_cast<T_To*>(source.data()),
                               source.size()
                              );
    }
}
