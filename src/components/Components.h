#pragma once

#include "Common.h"
#include "VuShader.h"

class Mesh;

struct MeshRenderer {
    Mesh* mesh;
    VuShader* vuShader;
    uint32 materialIndex;
};


struct Spinn {
    float3 Axis = float3(0, 1, 0);
    float Angle = glm::radians(0.f);
};

