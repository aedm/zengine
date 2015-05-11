#pragma once

#include "../base/vectormath.h"
#include <memory>
#include <string>

//class Mesh;
class Texture;

/// Macrolist for operator types with an output variable (name, output type, shader token)
#define VALUETYPE_LIST \
	ITEM(FLOAT,			float,		"float"		) \
	ITEM(VEC2,			Vec2,		"vec2"		) \
	ITEM(VEC3,			Vec3,		"vec3"		) \
	ITEM(VEC4,			Vec4,		"vec4"		) \
	ITEM(UINT,			UINT,		"uint"		) \
	ITEM(MATRIX44,		Matrix,		"matrix"	) \
	ITEM(TEXTURE,		Texture*,	"texture"	) \


/// Possible node types
enum class NodeType
{
	/// Node types holding a value type
	#undef ITEM
	#define ITEM(name, type, token) name,
	VALUETYPE_LIST

	/// Other node types 
	SHADER,
	SHADER_SOURCE,
	SHADER_STUB,
	MODEL,
	MESH,

	/// Empty stub value type
	NONE,

	/// Editor nodes
	UI,
	WIDGET,
	GRAPH,
	DOCUMENT,

	/// Slot type that allows all node connections
	ALLOW_ALL,

	/// Undefined behavior
	UNDEFINED
};

/// Variable sizes in bytes, indexed by NodeType (only VALUETYPE_LIST names)
extern const int VariableByteSizes[];

/// Variable names, indexed by NodeType (only VALUETYPE_LIST names)
extern const char* VariableNames[];


/// Type helpers
template<NodeType T> struct NodeTypes;
#undef ITEM
#define ITEM(name, type, token) template<> struct NodeTypes<NodeType::name> { typedef type Type; };
VALUETYPE_LIST

#undef ITEM
#define ITEM(name, type, token) typedef type name##_TYPE;
VALUETYPE_LIST


/// Macrolist for attribute types (name, type, token)
#define VERTEXATTRIBUTE_LIST \
	ITEM(POSITION,		NodeType::VEC3,		"aPosition"	) \
	ITEM(TEXCOORD,		NodeType::VEC2,		"aTexCoord"	) \
	ITEM(NORMAL,		NodeType::VEC3,		"aNormal"	) \
	ITEM(BINORMAL,		NodeType::VEC3,		"aBinormal"	) \


/// Possible variable types in vertex attributes
enum class VertexAttributeUsage
{
	#undef ITEM
	#define ITEM(name, type, token) name,
	VERTEXATTRIBUTE_LIST
	
	/// Number of possible vertex attributes
	COUNT
};

/// Array for attribute types, index by VertexAttributeEnum
extern const NodeType VertexAttributeType[];

/// Array for attribute names, index by VertexAttributeEnum
extern const char* VertexAttributeName[];




/// Additional types
typedef std::shared_ptr<std::string> SharedString;