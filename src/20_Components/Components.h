#pragma once

#include "10_Core/Common.h"

namespace Vu
{

struct VuMaterial;
struct VuMesh;

struct VuShader;

struct MeshRenderer
{
    VuMesh*                     mesh;
    std::shared_ptr<VuMaterial> materialHnd;
};


struct Spinn
{
    vec3  axis  = vec3(0, 1, 0);
    float angle = 0.1F;
};


}