#pragma once

namespace Vu::Math {

struct Float3;
struct Float4;
struct Quaternion;

struct Float4x4 {
  // Stored in column-major order: m[column][row]
  float m[4][4];

  // Constructor - identity matrix by default
  Float4x4();

  // Constructor with 16 floats (column-major)
  Float4x4(float m00,
           float m01,
           float m02,
           float m03,
           float m10,
           float m11,
           float m12,
           float m13,
           float m20,
           float m21,
           float m22,
           float m23,
           float m30,
           float m31,
           float m32,
           float m33);

  // Access elements
  float&
  operator()(int row, int col);

  const float&
  operator()(int row, int col) const;

  // Get column as Float4
  Float4
  getColumn(int col) const;

  // Set column from Float4
  void
  setColumn(int col, const Float4& vec);

  // Matrix multiplication
  Float4x4&
  operator*=(const Float4x4& rhs);
};

// Matrix operations
Float4x4
operator*(const Float4x4& lhs, const Float4x4& rhs);

// Matrix-vector multiplication (assumes w=1 for position vectors)
Float3
operator*(const Float4x4& mat, const Float3& vec);

// Matrix-vector multiplication
Float4
operator*(const Float4x4& mat, const Float4& vec);

// Matrix utility functions
Float4x4
transpose(const Float4x4& mat);

// Creates a translation matrix
Float4x4
createTranslation(const Float3& position);

// Creates a scaling matrix
Float4x4
createScale(const Float3& scale);

// Creates a rotation matrix from quaternion
Float4x4
createRotation(const Quaternion& quaternion);

// Creates a TRS (Translation-Rotation-Scale) matrix
Float4x4
createTRSMatrix(const Float3& position, const Quaternion& quaternion, const Float3& scale);

Float4x4
inverse(const Float4x4& mat);

} // namespace Vu::Math
