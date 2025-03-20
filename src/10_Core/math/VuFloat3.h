#pragma once

namespace Vu::Math {
    struct Float2;

    struct Float3 {
        float x;
        float y;
        float z;

        // Constructors
        Float3();

        Float3(float x, float y, float z);

        Float3(const Float2& xy, float z);

        // Basic operators
        Float3& operator+=(const Float3& rhs);

        Float3& operator-=(const Float3& rhs);

        Float3& operator*=(float scalar);

        Float3& operator/=(float scalar);

        // Conversion to Float2
        Float2 xy() const;
    };

    // Non-member operators for Float3
    Float3 operator+(Float3 lhs, const Float3& rhs);

    Float3 operator-(Float3 lhs, const Float3& rhs);

    Float3 operator*(Float3 vec, float scalar);

    Float3 operator*(float scalar, Float3 vec);

    Float3 operator/(Float3 vec, float scalar);

    Float3 operator-(const Float3& vec);

    // Float3 utility functions
    float length(const Float3& vec);

    float lengthSquared(const Float3& vec);

    Float3 normalize(const Float3& vec);

    float dot(const Float3& a, const Float3& b);

    Float3 cross(const Float3& a, const Float3& b);

    Float3 lerp(const Float3& a, const Float3& b, float t);
}
