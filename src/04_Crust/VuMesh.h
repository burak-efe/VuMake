#pragma once

#include <span>

#include "02_OuterCore/Common.h"
#include "02_OuterCore/VuCommon.h"

namespace Vu {
struct VuBuffer;

struct VuMesh {
  uint32_t                  m_vertexCount {};
  std::shared_ptr<VuBuffer> m_indexBuffer {};
  std::shared_ptr<VuBuffer> m_vertexBuffer {};

  static VkDeviceSize
  totalAttributesSizePerVertex();

  [[nodiscard]] VkDeviceSize
  getNormalOffsetAsByte() const;

  [[nodiscard]] VkDeviceSize
  getTangentOffsetAsByte() const;

  [[nodiscard]] VkDeviceSize
  getUV_OffsetAsByte() const;

  static void
  calculateTangents(const std::span<uint32_t> indices,
                    const std::span<float3>   positions,
                    const std::span<float3>   normals,
                    const std::span<float2>   uvs,
                    std::span<float4>         tangents);
};
} // namespace Vu
