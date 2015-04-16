#pragma once

#include "../base/vectormath.h"
#include <memory>
#include <string>

class Mesh;
class Texture;

/// Macrolist for operator types with an output variable (name, output type, shader token)
#define VALUETYPE_LIST \
	ITEM(FLOAT,			float,		"Float"		) \
	ITEM(VEC2,			Vec2,		"Vec2"		) \
	ITEM(VEC3,			Vec3,		"Vec3"		) \
	ITEM(VEC4,			Vec4,		"Vec4"		) \
	ITEM(UINT,			UINT,		"Uint"		) \
	ITEM(MATRIX44,		Matrix,		"Matrix"	) \
	ITEM(TEXTURE,		Texture*,	"Texture"	) \
	ITEM(MESH,			Mesh*,		"Mesh"		) \


/// Possible node types
enum class NodeType
{
	/// Node types with an output value
	#undef ITEM
	#define ITEM(name, type, token) name,
	VALUETYPE_LIST

	/// Node types without an output value
	SHADER,
	MODEL,

	/// Undefined behavior
	UNDEFINED
};

/// Macrolist for attribute types (name, type, token)
#define VERTEXATTRIBUTE_LIST \
	ITEM(VERTEXATTRIB_POSITION,		NodeType::VEC3,		"aPosition"	) \
	ITEM(VERTEXATTRIB_TEXCOORD,		NodeType::VEC2,		"aTexCoord"	) \
	ITEM(VERTEXATTRIB_NORMAL,		NodeType::VEC3,		"aNormal"	) \
	ITEM(VERTEXATTRIB_BINORMAL,		NodeType::VEC3,		"aBinormal"	) \


/// Array for variable sizes in bytes, index by OpTypeEnum (only OPTYPE_LIST names)
extern const int VariableByteSizes[];

/// Array for variable names, index by OpTypeEnum (only OPTYPE_LIST names)
extern const char* VariableNames[];


/// Possible variable types in vertex attributes
enum VertexAttributeEnum
{
	#undef ITEM
	#define ITEM(name, type, token) name,
	VERTEXATTRIBUTE_LIST
	
	/// Number of possible vertex attributes
	VERTEXATTRIB_TYPE_COUNT
};

/// Array for attribute types, index by VertexAttributeEnum
extern const NodeType VertexAttributeType[];

/// Array for attribute names, index by VertexAttributeEnum
extern const char* VertexAttributeName[];


/// Type helpers
template<NodeType T> struct OpTypes;
	#undef ITEM
	#define ITEM(name, type, token) template<> struct OpTypes<NodeType::name> { typedef type Type; };
	VALUETYPE_LIST

	#undef ITEM
	#define ITEM(name, type, token) typedef type name##_TYPE;
	VALUETYPE_LIST


/// Additional types
typedef std::shared_ptr<std::string> SharedString;