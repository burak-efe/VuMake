#include "VuFloat4x4.h"

#include <cmath>
#include <limits>

#include "VuFloat3.h"
#include "VuFloat4.h"
#include "VuQuaternion.h"

Vu::Math::Float4x4::Float4x4()
{
    m[0][0] = 1.0f;
    m[0][1] = 0.0f;
    m[0][2] = 0.0f;
    m[0][3] = 0.0f;
    m[1][0] = 0.0f;
    m[1][1] = 1.0f;
    m[1][2] = 0.0f;
    m[1][3] = 0.0f;
    m[2][0] = 0.0f;
    m[2][1] = 0.0f;
    m[2][2] = 1.0f;
    m[2][3] = 0.0f;
    m[3][0] = 0.0f;
    m[3][1] = 0.0f;
    m[3][2] = 0.0f;
    m[3][3] = 1.0f;
}

Vu::Math::Float4x4::Float4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21,
                             float m22, float m23, float m30, float m31, float m32, float m33)
{
    m[0][0] = m00;
    m[0][1] = m01;
    m[0][2] = m02;
    m[0][3] = m03;
    m[1][0] = m10;
    m[1][1] = m11;
    m[1][2] = m12;
    m[1][3] = m13;
    m[2][0] = m20;
    m[2][1] = m21;
    m[2][2] = m22;
    m[2][3] = m23;
    m[3][0] = m30;
    m[3][1] = m31;
    m[3][2] = m32;
    m[3][3] = m33;
}

float& Vu::Math::Float4x4::operator()(int row, int col)
{
    return m[col][row]; // Column-major: m[column][row]
}

const float& Vu::Math::Float4x4::operator()(int row, int col) const
{
    return m[col][row]; // Column-major: m[column][row]
}

Vu::Math::Float4 Vu::Math::Float4x4::getColumn(int col) const
{
    return Float4(m[col][0], m[col][1], m[col][2], m[col][3]);
}

void Vu::Math::Float4x4::setColumn(int col, const Float4& vec)
{
    m[col][0] = vec.x;
    m[col][1] = vec.y;
    m[col][2] = vec.z;
    m[col][3] = vec.w;
}

Vu::Math::Float4x4& Vu::Math::Float4x4::operator*=(const Float4x4& rhs)
{
    Float4x4 temp;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            temp.m[i][j] =
                m[0][j] * rhs.m[i][0] +
                m[1][j] * rhs.m[i][1] +
                m[2][j] * rhs.m[i][2] +
                m[3][j] * rhs.m[i][3];
        }
    }
    *this = temp;
    return *this;
}

Vu::Math::Float4x4 Vu::Math::operator*(const Float4x4& lhs, const Float4x4& rhs)
{
    Float4x4 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] =
                lhs.m[0][j] * rhs.m[i][0] +
                lhs.m[1][j] * rhs.m[i][1] +
                lhs.m[2][j] * rhs.m[i][2] +
                lhs.m[3][j] * rhs.m[i][3];
        }
    }
    return result;
}

Vu::Math::Float3 Vu::Math::operator*(const Float4x4& mat, const Float3& vec)
{
    Float3 result;
    result.x = mat.m[0][0] * vec.x + mat.m[1][0] * vec.y + mat.m[2][0] * vec.z + mat.m[3][0];
    result.y = mat.m[0][1] * vec.x + mat.m[1][1] * vec.y + mat.m[2][1] * vec.z + mat.m[3][1];
    result.z = mat.m[0][2] * vec.x + mat.m[1][2] * vec.y + mat.m[2][2] * vec.z + mat.m[3][2];
    return result;
}

Vu::Math::Float4 Vu::Math::operator*(const Float4x4& mat, const Float4& vec)
{
    Float4 result;
    result.x = mat.m[0][0] * vec.x + mat.m[1][0] * vec.y + mat.m[2][0] * vec.z + mat.m[3][0] * vec.w;
    result.y = mat.m[0][1] * vec.x + mat.m[1][1] * vec.y + mat.m[2][1] * vec.z + mat.m[3][1] * vec.w;
    result.z = mat.m[0][2] * vec.x + mat.m[1][2] * vec.y + mat.m[2][2] * vec.z + mat.m[3][2] * vec.w;
    result.w = mat.m[0][3] * vec.x + mat.m[1][3] * vec.y + mat.m[2][3] * vec.z + mat.m[3][3] * vec.w;
    return result;
}

Vu::Math::Float4x4 Vu::Math::transpose(const Float4x4& mat)
{
    Float4x4 result;
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.m[i][j] = mat.m[j][i];
        }
    }
    return result;
}

Vu::Math::Float4x4 Vu::Math::createTranslation(const Float3& position)
{
    Float4x4 result;
    result.m[3][0] = position.x;
    result.m[3][1] = position.y;
    result.m[3][2] = position.z;
    return result;
}

Vu::Math::Float4x4 Vu::Math::createScale(const Float3& scale)
{
    Float4x4 result;
    result.m[0][0] = scale.x;
    result.m[1][1] = scale.y;
    result.m[2][2] = scale.z;
    return result;
}

Vu::Math::Float4x4 Vu::Math::createRotation(const Quaternion& quaternion)
{
    float x = quaternion.x;
    float y = quaternion.y;
    float z = quaternion.z;
    float w = quaternion.w;

    float xx = x * x;
    float xy = x * y;
    float xz = x * z;
    float xw = x * w;

    float yy = y * y;
    float yz = y * z;
    float yw = y * w;

    float zz = z * z;
    float zw = z * w;

    Float4x4 result;

    result.m[0][0] = 1.0f - 2.0f * (yy + zz);
    result.m[0][1] = 2.0f * (xy + zw);
    result.m[0][2] = 2.0f * (xz - yw);
    result.m[0][3] = 0.0f;

    result.m[1][0] = 2.0f * (xy - zw);
    result.m[1][1] = 1.0f - 2.0f * (xx + zz);
    result.m[1][2] = 2.0f * (yz + xw);
    result.m[1][3] = 0.0f;

    result.m[2][0] = 2.0f * (xz + yw);
    result.m[2][1] = 2.0f * (yz - xw);
    result.m[2][2] = 1.0f - 2.0f * (xx + yy);
    result.m[2][3] = 0.0f;

    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;

    return result;
}

Vu::Math::Float4x4 Vu::Math::createTRSMatrix(const Float3& position, const Quaternion& quaternion, const Float3& scale)
{
    // Create rotation matrix
    Float4x4 rotationMatrix = createRotation(quaternion);

    // Scale the rotation matrix columns
    for (int i = 0; i < 3; i++)
    {
        rotationMatrix.m[i][0] *= scale.x;
        rotationMatrix.m[i][1] *= scale.y;
        rotationMatrix.m[i][2] *= scale.z;
    }

    // Set translation
    rotationMatrix.m[3][0] = position.x;
    rotationMatrix.m[3][1] = position.y;
    rotationMatrix.m[3][2] = position.z;

    return rotationMatrix;
}

Vu::Math::Float4x4 Vu::Math::inverse(const Float4x4& mat)
{
    Float4x4     inv;
    const float* m = &mat.m[0][0];

    float invOut[16];

    invOut[0] = m[5] * m[10] * m[15] -
                m[5] * m[11] * m[14] -
                m[9] * m[6] * m[15] +
                m[9] * m[7] * m[14] +
                m[13] * m[6] * m[11] -
                m[13] * m[7] * m[10];

    invOut[4] = -m[4] * m[10] * m[15] +
                m[4] * m[11] * m[14] +
                m[8] * m[6] * m[15] -
                m[8] * m[7] * m[14] -
                m[12] * m[6] * m[11] +
                m[12] * m[7] * m[10];

    invOut[8] = m[4] * m[9] * m[15] -
                m[4] * m[11] * m[13] -
                m[8] * m[5] * m[15] +
                m[8] * m[7] * m[13] +
                m[12] * m[5] * m[11] -
                m[12] * m[7] * m[9];

    invOut[12] = -m[4] * m[9] * m[14] +
                 m[4] * m[10] * m[13] +
                 m[8] * m[5] * m[14] -
                 m[8] * m[6] * m[13] -
                 m[12] * m[5] * m[10] +
                 m[12] * m[6] * m[9];

    invOut[1] = -m[1] * m[10] * m[15] +
                m[1] * m[11] * m[14] +
                m[9] * m[2] * m[15] -
                m[9] * m[3] * m[14] -
                m[13] * m[2] * m[11] +
                m[13] * m[3] * m[10];

    invOut[5] = m[0] * m[10] * m[15] -
                m[0] * m[11] * m[14] -
                m[8] * m[2] * m[15] +
                m[8] * m[3] * m[14] +
                m[12] * m[2] * m[11] -
                m[12] * m[3] * m[10];

    invOut[9] = -m[0] * m[9] * m[15] +
                m[0] * m[11] * m[13] +
                m[8] * m[1] * m[15] -
                m[8] * m[3] * m[13] -
                m[12] * m[1] * m[11] +
                m[12] * m[3] * m[9];

    invOut[13] = m[0] * m[9] * m[14] -
                 m[0] * m[10] * m[13] -
                 m[8] * m[1] * m[14] +
                 m[8] * m[2] * m[13] +
                 m[12] * m[1] * m[10] -
                 m[12] * m[2] * m[9];

    invOut[2] = m[1] * m[6] * m[15] -
                m[1] * m[7] * m[14] -
                m[5] * m[2] * m[15] +
                m[5] * m[3] * m[14] +
                m[13] * m[2] * m[7] -
                m[13] * m[3] * m[6];

    invOut[6] = -m[0] * m[6] * m[15] +
                m[0] * m[7] * m[14] +
                m[4] * m[2] * m[15] -
                m[4] * m[3] * m[14] -
                m[12] * m[2] * m[7] +
                m[12] * m[3] * m[6];

    invOut[10] = m[0] * m[5] * m[15] -
                 m[0] * m[7] * m[13] -
                 m[4] * m[1] * m[15] +
                 m[4] * m[3] * m[13] +
                 m[12] * m[1] * m[7] -
                 m[12] * m[3] * m[5];

    invOut[14] = -m[0] * m[5] * m[14] +
                 m[0] * m[6] * m[13] +
                 m[4] * m[1] * m[14] -
                 m[4] * m[2] * m[13] -
                 m[12] * m[1] * m[6] +
                 m[12] * m[2] * m[5];

    invOut[3] = -m[1] * m[6] * m[11] +
                m[1] * m[7] * m[10] +
                m[5] * m[2] * m[11] -
                m[5] * m[3] * m[10] -
                m[9] * m[2] * m[7] +
                m[9] * m[3] * m[6];

    invOut[7] = m[0] * m[6] * m[11] -
                m[0] * m[7] * m[10] -
                m[4] * m[2] * m[11] +
                m[4] * m[3] * m[10] +
                m[8] * m[2] * m[7] -
                m[8] * m[3] * m[6];

    invOut[11] = -m[0] * m[5] * m[11] +
                 m[0] * m[7] * m[9] +
                 m[4] * m[1] * m[11] -
                 m[4] * m[3] * m[9] -
                 m[8] * m[1] * m[7] +
                 m[8] * m[3] * m[5];

    invOut[15] = m[0] * m[5] * m[10] -
                 m[0] * m[6] * m[9] -
                 m[4] * m[1] * m[10] +
                 m[4] * m[2] * m[9] +
                 m[8] * m[1] * m[6] -
                 m[8] * m[2] * m[5];

    float det = m[0] * invOut[0] + m[1] * invOut[4] + m[2] * invOut[8] + m[3] * invOut[12];

    if (std::fabs(det) < std::numeric_limits<float>::epsilon())
    {
        // Non-invertible matrix; return identity as a fallback
        return Float4x4();
    }

    float invDet = 1.0f / det;

    for (int i = 0; i < 16; ++i)
    {
        reinterpret_cast<float*>(&inv.m[0][0])[i] = invOut[i] * invDet;
    }

    return inv;
}
