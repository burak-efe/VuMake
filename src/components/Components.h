#pragma once

#include "Common.h"
#include "VuPools.h"
#include "VuResourceManager.h"


namespace Vu {
    struct VuMesh;
    struct VuShader;

    struct MeshRenderer {
        VuMesh* mesh;
        VuHandle2<VuShader> shader;
        uint32 materialIndex;
    };


    struct Spinn {
        float3 axis = float3(0, 1, 0);
        float angle = 0.1F;
    };
}
