#include "VuMaterial.h"

#include "03_Mantle/VuGraphicsPipeline.h" // for VuGraphicsPipeline
#include "VuShader.h"                     // for VuShader

Vu::VuMaterial::VuMaterial() = default;

Vu::VuMaterial::VuMaterial(MaterialSettings                 matSettings,
                           const std::shared_ptr<VuShader>& shaderHnd,
                           const std::shared_ptr<u32>&      materialDataHnd)
    : materialSettings {matSettings}, shaderHnd {shaderHnd}, materialDataHnd {materialDataHnd} {
  VuGraphicsPipeline& unused = shaderHnd.get()->requestPipeline(materialSettings);
}
