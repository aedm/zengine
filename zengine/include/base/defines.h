#pragma once

#define GLM_FORCE_XYZW_ONLY
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

using glm::vec2;
using glm::ivec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

/// Shader stuff
const int		NoiseMapSize				= 512;

/// Unsigned types
#ifndef ULONG
typedef unsigned long	ULONG;
#endif
#ifndef UINT
typedef unsigned int	UINT;
#endif
#ifndef USHORT
typedef unsigned short	USHORT;
#endif
#ifndef UCHAR
typedef unsigned char	UCHAR;
#endif

/// Various
const float		Pi							= 3.14159265358979f;

/// API types
typedef			UINT						DrawingAPIHandle;	/// OpenGL uses UINTs

typedef			DrawingAPIHandle			VertexBufferHandle;
typedef			DrawingAPIHandle			IndexBufferHandle;
typedef			DrawingAPIHandle			ShaderHandle;
typedef			DrawingAPIHandle			VertexDeclaration;

typedef			int							UniformId;
typedef			int							AttributeId;
typedef			int							SamplerId;
typedef			UINT					  FrameBufferId;
typedef		  ULONG		        IndexEntry;


/// Other
#define OWNERSHIP
#define NOOWNERSHIP
#define OPTIONAL

template<typename T> 
void SafeDelete(T*& Object) { delete Object; Object = 0; }

template<typename T, int N>
constexpr UINT ElementCount(const T (&arr)[N]) { return N; }

/// Texel types
enum class TexelType
{
	ARGB8,
	ARGB16,
	ARGB16F,
	ARGB32F,
  DEPTH32F,
};

/// This is a debug value
extern bool PleaseNoNewResources;