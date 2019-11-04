#pragma once

#include "../base/defines.h"
#include <vector>
#include <memory>

// Possible types of shader values and variables
enum class ValueType {
  FLOAT,
  VEC2,
  VEC3,
  VEC4,
  MATRIX44,
};

UINT ValueTypeByteSize(ValueType type);


// Translation between shader types and C++ types
template<ValueType T> struct ValueTypes;
template<> struct ValueTypes<ValueType::FLOAT> { typedef float Type; };
template<> struct ValueTypes<ValueType::VEC2> { typedef vec2 Type; };
template<> struct ValueTypes<ValueType::VEC3> { typedef vec3 Type; };
template<> struct ValueTypes<ValueType::VEC4> { typedef vec4 Type; };
template<> struct ValueTypes<ValueType::MATRIX44> { typedef mat4 Type; };


/// Possible variable types in vertex attributes
enum class VertexAttributeUsage {
  POSITION,
  TEXCOORD,
  NORMAL,
  TANGENT,

  /// Number of possible vertex attributes
  COUNT,

  /// Undetermined
  NONE,
};

enum VertexAttributeMask {
  VERTEXATTRIB_POSITION_MASK = 1 << UINT(VertexAttributeUsage::POSITION),
  VERTEXATTRIB_TEXCOORD_MASK = 1 << UINT(VertexAttributeUsage::TEXCOORD),
  VERTEXATTRIB_NORMAL_MASK = 1 << UINT(VertexAttributeUsage::NORMAL),
  VERTEXATTRIB_TANGENT_MASK = 1 << UINT(VertexAttributeUsage::TANGENT),
};

/// Convert vertex attribute usage to value type
ValueType VertexAttributeUsageToValueType(VertexAttributeUsage usage);


/// An attribute of a vertex format, eg. position or UV
struct VertexAttribute {
  VertexAttributeUsage Usage;
  int Size;
  int Offset;
};

/// Describes the memory layout of a vertex format
class VertexFormat {
public:
  VertexFormat(UINT binaryFormat);

  bool HasAttribute(VertexAttributeUsage attribute) const;

  /// Size of all data of a single vertex in bytes
  int mStride;

  /// Bit-by-bit representation of vertex attributes
  UINT mBinaryFormat;

  /// List of vertex attributes
  std::vector<VertexAttribute> mAttributes;

  VertexAttribute* mAttributesArray[UINT(VertexAttributeUsage::COUNT)]{};
};


/// Common vertex formats
struct VertexPos {
  vec3 mPosition;

  /// Vertex format descriptor for this struct
  static std::shared_ptr<VertexFormat> mFormat;
};


struct VertexPosNorm {
  vec3 mPosition;
  vec3 mNormal;

  /// Vertex format descriptor for this struct
  static std::shared_ptr<VertexFormat> mFormat;
};

struct VertexPosUvNorm {
  vec3 mPosition;
  vec2 mUv;
  vec3 mNormal;

  /// Vertex format descriptor for this struct
  static std::shared_ptr<VertexFormat> mFormat;
};


struct VertexPosUvNormTangent {
  vec3 mPosition;
  vec2 mUv;
  vec3 mNormal;
  vec3 mTangent;

  /// Vertex format descriptor for this struct
  static std::shared_ptr<VertexFormat> mFormat;
};


struct VertexPosUv {
  vec3 mPosition;
  vec2 mUv;

  /// Vertex format descriptor for this struct
  static std::shared_ptr<VertexFormat> mFormat;
};
