#pragma once

#include "02_OuterCore/Common.h"

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
    float3  axis  = float3(0, 1, 0);
    float angle = 0.1F;
};


}