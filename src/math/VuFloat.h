#pragma once
#include <cmath>

namespace Vu::Math {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float RAD_TO_DEG = 180.0f / PI;
    constexpr float DEG_TO_RAD = PI / 180.0f;

    // Additional utility functions
    inline float clamp(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    inline float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    // Converts radians to degrees
    constexpr float toDegrees(float radians) {
        return radians * RAD_TO_DEG;
    }

    // Converts degrees to radians
    constexpr float toRadians(float degrees) {
        return degrees * DEG_TO_RAD;
    }
}
