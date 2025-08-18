#pragma once
#include <filesystem>

using path = std::filesystem::path;

namespace Vu::Math {
struct Float2;
struct Float3;
struct Float4;
struct Float4x4;
struct Quaternion;
} // namespace Vu::Math

using float2     = Vu::Math::Float2;
using float3     = Vu::Math::Float3;
using float4     = Vu::Math::Float4;
using float4x4   = Vu::Math::Float4x4;
using quaternion = Vu::Math::Quaternion;

constexpr uint32_t ZERO_FLAG = 0;
