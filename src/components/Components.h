#pragma once

#include "Common.h"

class Mesh;

struct MeshRenderer {
    Mesh* Mesh;
};


struct Spinn {
    float3 Axis = float3(0, 1, 0);
    float Angle = glm::radians(360.f);
};

