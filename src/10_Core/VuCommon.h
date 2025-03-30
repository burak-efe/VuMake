#pragma once

#include <source_location>
#include <span>

#include "volk.h"
#include "vk_mem_alloc.h"
#include "tracy/Tracy.hpp"
#include <cstdint>
#include <stdexcept>

namespace std::filesystem
{
    class path;
}

namespace Vu
{
    namespace Math
    {
        struct Float2;
        struct Float3;
        struct Float4;
        struct Float4x4;
        struct Quaternion;
    }

    using byte = std::byte;

    using u8  = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;

    using i8  = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;

    using vec2   = Math::Float2;
    using vec3   = Math::Float3;
    using vec4   = Math::Float4;
    using mat4x4 = Math::Float4x4;

    using quaternion = Math::Quaternion;

    using Path   = std::filesystem::path;
    using String = std::string;

    void VkCheck(VkResult res, std::source_location location = std::source_location::current());

    template <typename T_From, typename T_To>
    std::span<T_To> rpCastSpan(std::span<T_From> source)
    {
        static_assert(sizeof(T_From) == sizeof(T_To), "T_From and T_To must be the same size for reinterpret casting.");
        return std::span<T_To>(reinterpret_cast<T_To*>(source.data()), source.size());
    }
}


consteval std::uint8_t operator"" _ub(unsigned long long value)
{
    if (value > 255)
    {
        throw std::out_of_range("Value exceeds uint8_t range");
    }
    return static_cast<std::uint8_t>(value);
}

constexpr uint8_t val = 255_ub; //compile error not linting error
