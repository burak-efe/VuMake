#pragma once

#include <functional>
#include <optional>
#include <stack>

#include "VuCommon.h"

#include "01_InnerCore/TypeDefs.h"
#include "02_OuterCore/Common.h"
#include "02_OuterCore/FixedString.h"
#include "02_OuterCore/math/VuFloat4.h"
#include "02_OuterCore/math/VuFloat4x4.h"

namespace Vu {
using VuName = FixedString64;

struct GPU_Mesh {
  u32 vertexBufferHandle = {};
  u32 vertexCount        = {};
  u32 meshFlags          = {};
};

struct GPU_PBR_MaterialData {
  u32 texture0 = {};
  u32 texture1 = {};
  u32 texture2 = {};
  u32 texture3 = {};

  byte padding[48];
};

static_assert(sizeof(GPU_PBR_MaterialData) == 64);

struct GPU_PushConstant {
  mat4x4   trs               = {};
  u32      materialDataIndex = {};
  GPU_Mesh mesh              = {};
};

struct GPU_FrameConst {
  mat4x4 view        = {};
  mat4x4 proj        = {};
  mat4x4 inverseView = {};
  mat4x4 inverseProj = {};
  vec4   cameraPos   = {};
  vec4   cameraDir   = {};
  float  time        = {};
  float  debugIndex  = {};
};

struct VuDisposeStack {
  std::stack<std::function<void()>> disposeStack {};

  void
  push(const std::function<void()>& func) {
    disposeStack.push(func);
  }

  void
  disposeAll() {
    while (!disposeStack.empty()) {
      std::function<void()> disposeFunc = disposeStack.top();
      disposeFunc();
      disposeStack.pop();
    }
  }
};


} // namespace Vu
