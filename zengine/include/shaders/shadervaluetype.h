#pragma once

#include "../base/vectormath.h"
#include <vector>
#include <memory>

using namespace std;

// Possible types of shader values and variables
enum class ShaderValueType {
  FLOAT,
  VEC2,
  VEC3,
  VEC4,
  MATRIX44,
};

UINT ValueTypeByteSize(ShaderValueType type);


// Translation between shader types and C++ types
template<ShaderValueType T> struct ShaderValueTypes;
template<> struct ShaderValueTypes<ShaderValueType::FLOAT> { typedef float Type; };
template<> struct ShaderValueTypes<ShaderValueType::VEC2> { typedef Vec2 Type; };
template<> struct ShaderValueTypes<ShaderValueType::VEC3> { typedef Vec3 Type; };
template<> struct ShaderValueTypes<ShaderValueType::VEC4> { typedef Vec4 Type; };
template<> struct ShaderValueTypes<ShaderValueType::MATRIX44> { typedef Matrix Type; };


/// Macrolist for attribute types (name, type, token)
//#define VERTEXATTRIBUTE_LIST \
//	ITEM(POSITION,		ValueType::VEC3,		"aPosition"	) \
//	ITEM(TEXCOORD,		ValueType::VEC2,		"aTexCoord"	) \
//	ITEM(NORMAL,		  ValueType::VEC3,		"aNormal"	) \
//	ITEM(TANGENT,		  ValueType::VEC3,		"aTangent"	) \


/// Possible variable types in vertex attributes
enum class VertexAttributeUsage {
  //#undef ITEM
  //#define ITEM(name, type, token) name,
  //  VERTEXATTRIBUTE_LIST
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
  VERTEXATTRIB_POSITION_MASK = 1 << (UINT)VertexAttributeUsage::POSITION,
  VERTEXATTRIB_TEXCOORD_MASK = 1 << (UINT)VertexAttributeUsage::TEXCOORD,
  VERTEXATTRIB_NORMAL_MASK = 1 << (UINT)VertexAttributeUsage::NORMAL,
  VERTEXATTRIB_TANGENT_MASK = 1 << (UINT)VertexAttributeUsage::TANGENT,
  //VERTEXATTRIB_COLOR_MASK			= 1 << VERTEXATTRIB_COLOR
};

/// Convert vertex attribute usage to value type
ShaderValueType VertexAttributeUsageToValueType(VertexAttributeUsage usage);


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
  ~VertexFormat();

  bool HasAttribute(VertexAttributeUsage attrib);

  /// Size of all data of a single vertex in bytes
  int mStride;

  /// Bit-by-bit representation of vertex attributes
  UINT mBinaryFormat;

  /// List of vertex attributes
  vector<VertexAttribute> mAttributes;

  VertexAttribute* mAttributesArray[(UINT)VertexAttributeUsage::COUNT];
};


/// Common vertex formats
struct VertexPos {
  Vec3 position;

  /// Vertex format descriptor for this struct
  static shared_ptr<VertexFormat> format;
};


struct VertexPosNorm {
  Vec3 position;
  Vec3 normal;

  /// Vertex format descriptor for this struct
  static shared_ptr<VertexFormat> format;
};

struct VertexPosUVNorm {
  Vec3 position;
  Vec2 uv;
  Vec3 normal;

  /// Vertex format descriptor for this struct
  static shared_ptr<VertexFormat> format;
};


struct VertexPosUVNormTangent {
  Vec3 position;
  Vec2 uv;
  Vec3 normal;
  Vec3 tangent;

  /// Vertex format descriptor for this struct
  static shared_ptr<VertexFormat> format;
};


struct VertexPosUV {
  Vec3 position;
  Vec2 uv;

  /// Vertex format descriptor for this struct
  static shared_ptr<VertexFormat> format;
};
