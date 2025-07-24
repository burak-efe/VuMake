#pragma once

#include "02_OuterCore/Common.h"

namespace Vu {
struct VuBuffer;

struct VuMesh {
  uint32_t                  vertexCount  = {};
  std::shared_ptr<VuBuffer> indexBuffer  = {};
  std::shared_ptr<VuBuffer> vertexBuffer = {};

  static vk::DeviceSize totalAttributesSizePerVertex();

  [[nodiscard]] vk::DeviceSize getNormalOffsetAsByte() const;

  [[nodiscard]] vk::DeviceSize getTangentOffsetAsByte() const;

  [[nodiscard]] vk::DeviceSize getUV_OffsetAsByte() const;

  static void calculateTangents(const std::span<uint32_t> indices,
                                const std::span<float3>   positions,
                                const std::span<float3>   normals,
                                const std::span<float2>   uvs,
                                std::span<float4>         tangents);
};
} // namespace Vu
