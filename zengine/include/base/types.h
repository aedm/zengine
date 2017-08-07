#pragma once

#include "../base/vectormath.h"
#include <memory>
#include <string>

//class Mesh;
//class Texture;

/// Macrolist for operator types with an output variable (name, output type, shader token)
#define VALUETYPE_LIST \
	ITEM(FLOAT,			float	    ) \
	ITEM(VEC2,			Vec2		  ) \
	ITEM(VEC3,			Vec3		  ) \
	ITEM(VEC4,			Vec4		  ) \
	ITEM(UINT,			UINT		  ) \
	ITEM(MATRIX44,	Matrix		) \
	//ITEM(TEXTURE,		Texture*	) \


/// Variable sizes in bytes, indexed by NodeType (only VALUETYPE_LIST names)
extern const int gVariableByteSizes[];


/// Macrolist for attribute types (name, type, token)
#define VERTEXATTRIBUTE_LIST \
	ITEM(POSITION,		NodeType::VEC3,		"aPosition"	) \
	ITEM(TEXCOORD,		NodeType::VEC2,		"aTexCoord"	) \
	ITEM(NORMAL,		  NodeType::VEC3,		"aNormal"	) \
	ITEM(TANGENT,		  NodeType::VEC3,		"aTangent"	) \


/// Possible variable types in vertex attributes
enum class VertexAttributeUsage {
#undef ITEM
#define ITEM(name, type, token) name,
  VERTEXATTRIBUTE_LIST

  /// Number of possible vertex attributes
  COUNT
};


/// Array for attribute names, index by VertexAttributeEnum
extern const char* gVertexAttributeName[];


/// Additional types
typedef std::shared_ptr<std::string> SharedString;
