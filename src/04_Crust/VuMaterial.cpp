#include "VuMaterial.h"

#include "VuShader.h" // for VuShader

namespace Vu {
struct VuGraphicsPipeline;
} // namespace Vu
struct VuMaterialDataHandle;

Vu::VuMaterial::VuMaterial() = default;

Vu::VuMaterial::VuMaterial(MaterialSettings                             matSettings,
                           const std::shared_ptr<VuShader>&             shaderHnd,
                           const std::shared_ptr<VuMaterialDataHandle>& materialDataHnd)
    : m_materialSettings {matSettings}, m_shaderHnd {shaderHnd}, m_materialDataHnd {materialDataHnd} {
  VuGraphicsPipeline& unused = shaderHnd.get()->requestPipeline(m_materialSettings);
}
