#include "VuMesh.h"

#include "02_OuterCore/VuCommon.h"

#include "01_InnerCore/TypeDefs.h"
#include "02_OuterCore/math/VuFloat2.h"
#include "02_OuterCore/math/VuFloat3.h"
#include "02_OuterCore/math/VuFloat4.h"

namespace Vu {

vk::DeviceSize VuMesh::totalAttributesSizePerVertex() {
  // pos, norm, tan , uv
  return sizeof(float3) + sizeof(float3) + sizeof(float4) + sizeof(float2);
}
vk::DeviceSize VuMesh::getNormalOffsetAsByte() const { return sizeof(float3) * vertexCount; }
vk::DeviceSize VuMesh::getTangentOffsetAsByte() const { return (sizeof(float3) + sizeof(float3)) * vertexCount; }
vk::DeviceSize VuMesh::getUV_OffsetAsByte() const {
  return (sizeof(float3) + sizeof(float3) + sizeof(float4)) * vertexCount;
}
void VuMesh::calculateTangents(const std::span<u32>    indices,
                               const std::span<float3> positions,
                               const std::span<float3> normals,
                               const std::span<float2> uvs,
                               std::span<float4>       tangents) {

  u32 vertexCount   = positions.size();
  u32 triangleCount = indices.size() / 3;

  std::vector<float3> tan1(vertexCount);
  std::vector<float3> tan2(vertexCount);

  for (u32 a = 0; a < triangleCount; a++) {
    u32 i1 = indices[a * 3 + 0];
    u32 i2 = indices[a * 3 + 1];
    u32 i3 = indices[a * 3 + 2];

    const float3 v1 = positions[i1];
    const float3 v2 = positions[i2];
    const float3 v3 = positions[i3];

    const float2 w1 = uvs[i1];
    const float2 w2 = uvs[i2];
    const float2 w3 = uvs[i3];

    float x1 = v2.x - v1.x;
    float x2 = v3.x - v1.x;
    float y1 = v2.y - v1.y;
    float y2 = v3.y - v1.y;
    float z1 = v2.z - v1.z;
    float z2 = v3.z - v1.z;

    float s1 = w2.x - w1.x;
    float s2 = w3.x - w1.x;
    float t1 = w2.y - w1.y;
    float t2 = w3.y - w1.y;

    float r = 1.0f / (s1 * t2 - s2 * t1);

    float3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);

    float3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);

    tan1[i1] += sdir;
    tan1[i2] += sdir;
    tan1[i3] += sdir;

    tan2[i1] += tdir;
    tan2[i2] += tdir;
    tan2[i3] += tdir;
  }

  for (u32 a = 0; a < vertexCount; a++) {
    float3 n   = normals[a];
    float3 t   = tan1[a];
    // Gram-Schmidt orthogonalize
    float3 vec = Math::normalize(t - n * Math::dot(n, t));
    float  sign;
    float  handedness = Math::dot(Math::cross(n, t), tan2[a]);
    if (handedness < 0.0F)
      sign = -1.0F;
    else
      sign = 1.0F;
    tangents[a] = float4(vec, sign);
  }
}
} // namespace Vu