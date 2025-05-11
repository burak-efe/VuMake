#include "VuMaterial.h"

#include "03_Mantle/VuGraphicsPipeline.h" // for VuGraphicsPipeline
#include "14_VuMake/VuShader.h"                // for VuShader

Vu::VuMaterial::VuMaterial() = default;

Vu::VuMaterial::VuMaterial(VuDevice*                        vuDevice,
                           MaterialSettings                 matSettings,
                           const std::shared_ptr<VuShader>& shaderHnd,
                           const std::shared_ptr<u32>&      materialDataHnd)
    : vuDevice{vuDevice},
      materialSettings{matSettings},
      shaderHnd{shaderHnd},
      materialDataHnd{materialDataHnd}
{
    auto unused = shaderHnd.get()->requestPipeline(materialSettings);
}
