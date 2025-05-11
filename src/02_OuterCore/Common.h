#pragma once

#include <filesystem>

using path = std::filesystem::path;

// template <typename T, typename E>
// using expected = tl::expected<T, E>;


namespace Vu {
namespace Math {
struct Float2;
struct Float3;
struct Float4;
struct Float4x4;
struct Quaternion;
}


using vec2   = Math::Float2;
using vec3   = Math::Float3;
using vec4   = Math::Float4;
using mat4x4 = Math::Float4x4;

using quaternion = Math::Quaternion;


}
