#pragma once

#include "02_OuterCore/Common.h"
#include "02_OuterCore/math/VuFloat4x4.h"
#include "02_OuterCore/math/VuQuaternion.h"

namespace Vu {
    struct Transform {
        float3     position = {0, 0, 0};
        quaternion rotation = quaternion::identity();
        float3     scale    = {1, 1, 1};

        void Rotate(const float3& axis, float angle) {
            rotation = Math::rotateOnAxis(rotation, axis, angle);
        }

        float4x4 ToTRS() {
            return createTRSMatrix(position, rotation, scale);
        }


        void SetEulerAngles(const float3& eulerAngles) {
            rotation = Math::fromEulerYXZ(eulerAngles);
        }
    };
}
