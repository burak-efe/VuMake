#pragma once

#include "Common.h"
#include "VuShader.h"

class Mesh;

namespace Vu {
    struct MeshRenderer {
        VuMesh* mesh;
        VuShader* vuShader;
        uint32 materialIndex;
    };


    struct Spinn {
        float3 axis = float3(0, 1, 0);
        float angle = glm::radians(0.f);
    };
}
