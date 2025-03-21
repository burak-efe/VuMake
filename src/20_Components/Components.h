#pragma once

#include "10_Core/VuCommon.h"
#include "12_VuMakeCore/VuPools.h"

namespace Vu {
    struct VuMesh;
    struct VuShader;

    struct MeshRenderer {
        VuMesh*             mesh;
        VuHandle2<VuShader> shader;
        uint32              materialIndex;
    };


    struct Spinn {
        float3 axis = float3(0, 1, 0);
        float angle = 0.1F;
    };
}
