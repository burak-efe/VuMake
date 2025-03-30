#pragma once

#include "10_Core/VuCommon.h"
#include "10_Core/math/VuQuaternion.h"

namespace Vu {
    struct Transform {
        vec3     position = {0, 0, 0};
        quaternion rotation = quaternion::identity();
        vec3     scale    = {1, 1, 1};

        void Rotate(const vec3& axis, float angle) {
            rotation = Math::rotateOnAxis(rotation, axis, angle);
        }

        mat4x4 ToTRS() {
            return createTRSMatrix(position, rotation, scale);
        }


        void SetEulerAngles(const vec3& eulerAngles) {
            rotation = Math::fromEulerYXZ(eulerAngles);
        }
    };
}
