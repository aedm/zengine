#pragma once

#include "../base/vectormath.h"
#include <memory>
#include <string>

class Mesh;
class Texture;

/// Macrolist for operator types with an output variable (name, output type, shader token)
#define NODETYPE_LIST \
	ITEM(NODE_FLOAT,				float,		"float"		) \
	ITEM(NODE_VEC2,					Vec2,		"vec2"		) \
	ITEM(NODE_VEC3,					Vec3,		"vec3"		) \
	ITEM(NODE_VEC4,					Vec4,		"vec4"		) \
	ITEM(NODE_UINT,					UINT,		"uint"		) \
	ITEM(NODE_MATRIX44,				Matrix,		"matrix"	) \
	ITEM(NODE_TEXURE,				Texture*,	"Texture"	) \
	ITEM(NODE_MESH,					Mesh*,		"Mesh"		) \


/// Macrolist for attribute types (name, type, token)
#define VERTEXATTRIBUTE_LIST \
	ITEM(VERTEXATTRIB_POSITION,		NODE_VEC3,	"aPosition"	) \
	ITEM(VERTEXATTRIB_TEXCOORD,		NODE_VEC2,	"aTexCoord"	) \
	ITEM(VERTEXATTRIB_NORMAL,		NODE_VEC3,	"aNormal"	) \
	ITEM(VERTEXATTRIB_BINORMAL,		NODE_VEC3,	"aBinormal"	) \


/// Possible operator types
enum NodeTypeEnum
{
	/// Operator types with an output value
	#undef ITEM
	#define ITEM(name, type, token) name,
	NODETYPE_LIST

	/// Operator types without an output value
	OP_SHADER,
	OP_MODEL,

	/// Undefined behavior
	OP_UNDEFINED
};

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
extern const NodeTypeEnum VertexAttributeType[];

/// Array for attribute names, index by VertexAttributeEnum
extern const char* VertexAttributeName[];


/// Type helpers
template<NodeTypeEnum T> struct OpTypes;
	#undef ITEM
	#define ITEM(name, type, token) template<> struct OpTypes<name> { typedef type Type; };
	NODETYPE_LIST

	#undef ITEM
	#define ITEM(name, type, token) typedef type name##_TYPE;
	NODETYPE_LIST


/// Additional types
typedef std::shared_ptr<std::string> SharedString;