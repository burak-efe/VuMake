#pragma once

#include <climits>
#include <cmath>
#include <cstdint>
#include <deque>
#include <expected>
#include <filesystem>
#include <format>
#include <immintrin.h>
#include <iostream>
#include <map>
#include <memory_resource>
#include <mutex>
#include <source_location>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "02_OuterCore/FixedString.h"
//#include "02_OuterCore/VuCommon.h"
//#include "03_Mantle/VuDevice.h"
//#include "03_Mantle/VuInstance.h"
//
#include "fastgltf/core.hpp"
#include "fastgltf/math.hpp"
#include "fastgltf/types.hpp"
//
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"
#include "imgui_internal.h"
#include "SDL3/SDL.h"
//
// #include "vulkan/vulkan.hpp"
// #include "vulkan/vulkan_raii.hpp"

// basic tempates
template class std::span<const char*>;
template class std::optional<unsigned int>;
template class std::basic_string<char>;
template class std::basic_format_string<char, const char*, unsigned int>;
template class std::vector<const char*>;

// vk templates
// template class std::span<vk::DescriptorSetLayout>;
// template class std::vector<VkValidationFeatureEnableEXT>;
// template class std::span<const vk::SurfaceFormatKHR>;
// template class std::is_default_constructible<std::deque<std::function<void()>>>;
// template class std::is_nothrow_default_constructible<std::pmr::polymorphic_allocator<unsigned int>>;
//
// template class std::shared_ptr<Vu::VuDevice>;
// template class std::shared_ptr<Vu::VuInstance>;
// template class std::shared_ptr<Vu::VuPhysicalDevice>;
