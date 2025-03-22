#pragma once

#include <bit>
#include <climits>
#include <cstdint>
#include <limits>

//Size Sanity Checks
static_assert(CHAR_BIT == 8, "[INSANE ENVIRONMENT]: a byte is not 8 bits!");
static_assert(sizeof(float) == 4, "[INSANE ENVIRONMENT]: float not 4 bytes");
static_assert(sizeof(double) == 8, "[INSANE ENVIRONMENT]: double not 8 bytes");

static_assert(sizeof(void*) == sizeof(size_t), "[INSANE ENVIRONMENT]: size_t size doesn't match pointer size");
static_assert(sizeof(size_t) == sizeof(uint64_t), "[INSANE ENVIRONMENT]: size_t size doesn't match uint64_t size");
static_assert(sizeof(std::uintptr_t) == sizeof(void*), "[INSANE ENVIRONMENT]: uintptr_t doesn't match pointer size");

//Other Sanity Checks
static_assert(std::numeric_limits<float>::is_iec559, "[INSANE ENVIRONMENT]: float is not IEEE 754");
static_assert(std::numeric_limits<double>::is_iec559, "[INSANE ENVIRONMENT]: double is not IEEE 754");
static_assert(std::endian::native == std::endian::little, "[INSANE ENVIRONMENT]: not a little-endian platform");
static_assert((~uint32_t(0)) == UINT32_MAX, "[INSANE ENVIRONMENT]: not two's complement");
static_assert(alignof(void*) == alignof(std::uintptr_t), "[INSANE ENVIRONMENT]: void* alignment mismatch");
static_assert(sizeof(time_t) >= 8, "[INSANE ENVIRONMENT]: time_t smaller than 8 byte (year 2038 problem?)");