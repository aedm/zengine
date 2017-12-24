#pragma once

#include "../base/vectormath.h"
#include <memory>
#include <string>

class Texture;

/// Macrolist for operator types with an output variable (name, output type, shader token)
#define VALUETYPE_LIST \
	ITEM(FLOAT, Float, float) \
	ITEM(VEC2, Vec2, Vec2) \
	ITEM(VEC3, Vec3, Vec3) \
	ITEM(VEC4, Vec4, Vec4) \
	ITEM(UINT, Uint, UINT) \
	ITEM(MATRIX44, Matrix, Matrix) \
	ITEM(STRING, String, std::string) \
	ITEM(TEXTURE, Texture, Texture*) \

/// Value types
enum class ValueType {
#undef ITEM
#define ITEM(name, capitalizedName, type) name,
  VALUETYPE_LIST
  NONE
};


/// Variable sizes in bytes, indexed by ValueType 
extern const int gVariableByteSizes[];


/// Macrolist for attribute types (name, type, token)
#define VERTEXATTRIBUTE_LIST \
	ITEM(POSITION,		ValueType::VEC3,		"aPosition"	) \
	ITEM(TEXCOORD,		ValueType::VEC2,		"aTexCoord"	) \
	ITEM(NORMAL,		  ValueType::VEC3,		"aNormal"	) \
	ITEM(TANGENT,		  ValueType::VEC3,		"aTangent"	) \


/// Possible variable types in vertex attributes
enum class VertexAttributeUsage {
#undef ITEM
#define ITEM(name, type, token) name,
  VERTEXATTRIBUTE_LIST

  /// Number of possible vertex attributes
  COUNT,

  /// Undetermined
  NONE,
};


/// Array for attribute names, index by VertexAttributeEnum
extern const char* gVertexAttributeName[];


/// Additional types
typedef std::shared_ptr<std::string> SharedString;
