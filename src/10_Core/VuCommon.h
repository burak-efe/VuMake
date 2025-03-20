#pragma once

#include <source_location>
#include <cstdint>
#include <filesystem>
#include <span>

#include "volk.h"
#include "vk_mem_alloc.h"
#include "tracy/Tracy.hpp"

#include "math/VuFloat2.h"
#include "math/VuFloat3.h"
#include "math/VuFloat4.h"
#include "math/VuFloat4x4.h"
#include "math/VuQuaternion.h"


namespace Vu
{
    using byte = std::byte;

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

    template <typename T>
    using unique_ptr = std::unique_ptr<T>;

    template <typename T>
    using span = std::span<T>;

    __declspec(noinline) void VkCheck(VkResult res, std::source_location location = std::source_location::current());

    template <typename T_From, typename T_To>
    std::span<T_To> rpCastSpan(std::span<T_From> source)
    {
        static_assert(sizeof(T_From) == sizeof(T_To), "T_From and T_To must be the same size for reinterpret casting.");
        return std::span<T_To>(reinterpret_cast<T_To*>(source.data()), source.size());
    }
}
