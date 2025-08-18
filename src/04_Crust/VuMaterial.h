#pragma once

#include <cstddef>     // for size_t
#include <functional>  // for hash
#include <memory>      // for shared_ptr
#include <string_view> // for hash
#include <variant>

#include "02_OuterCore/VuCommon.h"
#include "03_Mantle/VuTypes.h"
#include "InteroptStructs.h"

namespace Vu {
struct VuShader;

struct MaterialSettings {
  bool            isTransparent = false;
  VkCullModeFlags cullMode      = VK_CULL_MODE_BACK_BIT;

  friend bool
  operator==(const MaterialSettings& lhs, const MaterialSettings& rhs) {
    return lhs.isTransparent == rhs.isTransparent && lhs.cullMode == rhs.cullMode;
  }

  friend bool
  operator!=(const MaterialSettings& lhs, const MaterialSettings& rhs) {
    return !(lhs == rhs);
  }

  friend std::size_t
  hash_value(const MaterialSettings& obj) {
    std::size_t seed = 0x305407C8;
    seed ^= (seed << 6) + (seed >> 2) + 0x42B03DC4 + static_cast<std::size_t>(obj.isTransparent);
    seed ^= (seed << 6) + (seed >> 2) + 0x29CD679B + static_cast<uint32_t>(obj.cullMode);
    return seed;
  }
};
// ####################################################################################################################
//  Material owns the pipeline, uses shared material data
//  when parent shader recompiled, it should be recompiled too
struct VuMaterial {
  MaterialSettings                           m_materialSettings {};
  std::shared_ptr<VuShader>                  m_shaderHnd {};
  std::shared_ptr<GPU::VuMaterialDataHandle> m_materialDataHnd {};

  VuMaterial();
  VuMaterial(MaterialSettings                                  matSettings,
             const std::shared_ptr<VuShader>&                  shaderHnd,
             const std::shared_ptr<GPU::VuMaterialDataHandle>& materialDataHnd);
};
} // namespace Vu

// #####################################################################################################################
namespace std {
template <> struct hash<Vu::MaterialSettings> {
  std::size_t
  operator()(const Vu::MaterialSettings& settings) const noexcept {
    std::size_t h1 = std::hash<bool>()(settings.isTransparent);
    std::size_t h2 = std::hash<uint32_t>()(static_cast<uint32_t>(settings.cullMode));

    return h1 ^ (h2 << 1);
  }
};
} // namespace std
