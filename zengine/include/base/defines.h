#pragma once

/// Shader stuff
const int		MaxRegisterCount			= 64*4;
const int		MaxSamplerCount				= 8;

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

/// Matrix stack
const int		MatrixStackDepth			= 64;

/// Various
const float		Pi							= 3.14159265358979f;

const int		MaxIndexCount				= 60000;

const int		MaxSubstringLength			= 255;
const int		MaxUniformNameLength		= 255;

/// API types
typedef			UINT						DrawingAPIHandle;	/// OpenGL uses UINTs

typedef			DrawingAPIHandle			TextureHandle;
typedef			DrawingAPIHandle			VertexBufferHandle;
typedef			DrawingAPIHandle			IndexBufferHandle;
typedef			DrawingAPIHandle			ShaderHandle;
typedef			DrawingAPIHandle			VertexDeclaration;

typedef			int							UniformId;
typedef			int							AttributeId;
typedef			int							SamplerId;


/// Other
#define OWNERSHIP
#define NOOWNERSHIP
#define OPTIONAL

#ifndef foreach
#	define foreach BOOST_FOREACH
#endif

template<typename T> 
void SafeDelete(T*& Object) { delete Object; Object = 0; }

template<typename T, int N>
UINT ElementCount(const T (&arr)[N]) { return N; }

#define VERTEXSHADERVERSION "vs_3_0"
#define PIXELSHADERVERSION "ps_3_0"
#define INDEXBUFFERFORMAT D3DFMT_INDEX32
#define AUTODEPTHSTENCIL D3DFMT_D24X8
typedef		ULONG		IndexEntry;


/// Texel types
enum TexelTypeEnum
{
	TEXELTYPE_VOID,
	TEXELTYPE_RGBA_UINT8,
	TEXELTYPE_RGBA_UINT16,
	TEXELTYPE_RGBA_FLOAT16,
	TEXELTYPE_RGBA_FLOAT32,
};
