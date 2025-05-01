#pragma once

#include <source_location>
#include <filesystem>

#include "volk.h"
#include "vk_mem_alloc.h"
#include "tracy/Tracy.hpp"
#include "../08_LangUtils/TypeDefs.h"

using path = std::filesystem::path;

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


    using vec2   = Math::Float2;
    using vec3   = Math::Float3;
    using vec4   = Math::Float4;
    using mat4x4 = Math::Float4x4;

    using quaternion = Math::Quaternion;

    void VkCheck(VkResult res, std::source_location location = std::source_location::current());
}
