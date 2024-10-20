#pragma once

#include <iostream>
#include <print>
#include <string>
#include <format>
#include <cstdint>
#include <cassert>
#include <cstdlib>
#include <array>
#include <vector>

#include "glm/fwd.hpp"
#include "glm/mat4x4.hpp"
#include "glm/detail/type_quat.hpp"

#include "volk.h"
#include "VkBootstrap.h"
#include "vk_mem_alloc.h"



using uint8 = uint8_t;
using uint16 = uint16_t;
using uint32 = uint32_t;
using uint64 = uint64_t;

using int8 = int8_t;
using int16 = int16_t;
using int32 = int32_t;


using float2 = glm::vec2;
using float3 = glm::vec3;
using float4 = glm::vec4;
using quat = glm::quat;
using float4x4 = glm::mat4;


using array = std::array;
using vector = std::vector;