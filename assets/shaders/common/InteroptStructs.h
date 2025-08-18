#pragma once

#ifdef __cplusplus
#include "02_OuterCore/Common.h"
#include "02_OuterCore/math/VuFloat3.h"
#include "02_OuterCore/math/VuFloat4.h"
#include "02_OuterCore/math/VuFloat4x4.h"
#endif

namespace GPU {
enum ShaderDebugMode {
  None,
  NormalWS,
  PosWS,
  Depth,
};

// Point Light Structure
struct PointLight {
  float3 color     = {1, 1, 1}; // RGB intensity (linear space)
  float  intensity = 1;         // Scalar multiplier for brightness

  float3 position = {0, 1, 0}; // World-space position
  float  range    = 10;        // Maximum effective distance
};

// Directional Light Structure
struct DirectionalLight {
  float3 color;     // RGB intensity (linear space)
  float  intensity; // Scalar multiplier for brightness

  float3 direction; // Light direction (normalized, world-space)
};

struct Camera {
  float4x4 view;
  float4x4 proj;
  float4x4 inverseView;
  float4x4 inverseProj;
  float4   position;
  float4   direction;
  float    exposureScale = 1;
};

struct Mesh {
  uint32_t vertex_buffer_handle;
  uint32_t vertex_count;
  uint32_t mesh_flags;

#ifndef __cplusplus
  Ptr<float3>
  getPositionPtr() {
    return (Ptr<float3>)globalStorageBuffers[vertex_buffer_handle];
  }

  Ptr<float3>
  getNormalPtr() {
    uint64_t prev = (uint64_t)getPositionPtr();
    uint64_t p    = prev + sizeof(float3) * vertex_count;
    return (Ptr<float3>)p;
  }

  Ptr<float4>
  getTangentPtr() {
    uint64_t prev = (uint64_t)getNormalPtr();
    uint64_t p    = prev + sizeof(float3) * vertex_count;
    return (Ptr<float4>)p;
  }

  Ptr<float2>
  getUV_Ptr() {
    uint64_t prev = (uint64_t)getTangentPtr();
    uint64_t p    = prev + sizeof(float4) * vertex_count;
    return (Ptr<float2>)p;
  }
#endif
};

struct VuMaterialDataHandle {
  uint32_t index;
};

struct FrameConstant {
  Camera          camera;
  float           time;
  ShaderDebugMode debugIndex;
  PointLight      pointLights[2];
};

struct PushConstant {
  float4x4             model;
  VuMaterialDataHandle materialDataHandle;
  Mesh                 mesh;
};

struct MatData_Raw {
  uint32_t data[16];
};

struct MatData_PbrDeferred {
  uint32_t colorTexture;
  uint32_t normalTexture;
  uint32_t aoRoughMetalTexture;
  uint32_t worldSpacePosTexture;
  uint32_t depthTexture;

  uint32_t padding[11];
};

} // namespace GPU