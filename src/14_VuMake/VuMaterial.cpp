#include "VuMaterial.h"

#include "11_Config/VuCtx.h"
#include "VuDevice.h"

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
