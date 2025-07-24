#pragma once

#if defined(__cplusplus)
#include "02_OuterCore/Common.h"
#include "02_OuterCore/math/VuFloat3.h"
#include "02_OuterCore/math/VuFloat4.h"
#include "02_OuterCore/math/VuFloat4x4.h"
#endif

enum ShaderDebugMode {
  None,
  NormalWS,
  PosWS,
  Depth,
};

// Point Light Structure
struct PointLight {
  float3 color;     // RGB intensity (linear space)
  float  intensity; // Scalar multiplier for brightness

  float3 position; // World-space position
  float  range;    // Maximum effective distance
};

// Directional Light Structure
struct DirectionalLight {
  float3 color;     // RGB intensity (linear space)
  float  intensity; // Scalar multiplier for brightness

  float3 direction; // Light direction (normalized, world-space)
};

struct FrameConst_RawData {
  float4x4        view;
  float4x4        proj;
  float4x4        inverseView;
  float4x4        inverseProj;
  float4          cameraPos;
  float4          cameraDir;
  float           time;
  ShaderDebugMode debugIndex;
  PointLight      pointLights[2];
};

struct Mesh_RawData {
  uint32_t vertex_buffer_handle;
  uint32_t vertex_count;
  uint32_t mesh_flags;
};

struct VuMaterialDataHandle {
  uint32_t index;
};

struct PushConsts_RawData {
  float4x4             model;
  VuMaterialDataHandle material_data_handle;
  Mesh_RawData         mesh_rd;
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
