#pragma once

#include "../base/defines.h"
#include "../base/helpers.h"
#include "../base/vectormath.h"
#include "../render/drawingapi.h"
#include "shaderMetadata.h"
#include <vector>

using namespace std;

struct Uniform;
struct Sampler;
struct ShaderLocal;
struct ShaderGlobal;
class UniformBlock;
class Texture;
struct Globals;
class ShaderSource;

extern const EnumMapperA GlobalUniformMapper[];
extern const int GlobalUniformOffsets[];

/// Macrolist for global uniforms (name, type, variable/token)
#define GLOBALUSAGE_LIST \
	ITEM(USAGE_TIME,					FLOAT,			Time)					\
	ITEM(USAGE_MATRIX_VIEW,				MATRIX44,		View)					\
	ITEM(USAGE_MATRIX_PROJECTION,		MATRIX44,		Projection)				\
	ITEM(USAGE_MATRIX_TRANSFORMATION,	MATRIX44,		Transformation)			\
	ITEM(USAGE_NOISEMAP_SIZE,			VEC2,			NoiseMapSize)			\
	ITEM(USAGE_NOISEMAP_SIZE_RECIP,		VEC2,			NoiseMapSizeRecip)		\
	ITEM(USAGE_RENDERTARGET_SIZE,		VEC2,			RenderTargetSize)		\
	ITEM(USAGE_RENDERTARGET_SIZE_RECIP,	VEC2,			RenderTargetSizeRecip)	\
	ITEM(USAGE_VIEWPORT_SIZE,			VEC2,			ViewportSize)			\
	ITEM(USAGE_PIXEL_SIZE,				VEC2,			PixelSize)				\
	ITEM(USAGE_DIFFUSE_COLOR,			VEC4,			DiffuseColor)			\
	ITEM(USAGE_AMBIENT_COLOR,			VEC4,			AmbientColor)			\
	ITEM(USAGE_DEPTH_BIAS,				FLOAT,			DepthBias)				\


/// Shader program. All shader stages together, compiled, linked.
class Shader
{
public:
	Shader(ShaderHandle Handle, const vector<UniformBlock*>& UniformBlocks, 
		const vector<Sampler*>& Samplers, const CompileSettings* Settings,
		const vector<ShaderAttributeDesc>& Attributes, const shared_ptr<ShaderSource>& Source);

	~Shader();

	/// Sets active shader through the graphics API 
	void							Set() const;

	/// Resource ID assigned by API
	const ShaderHandle				Handle;

	/// List of uniform arrays
	const vector<UniformBlock*>		UniformBlocks;

	/// List of samplers
	const vector<Sampler*>			Samplers;

	/// Metadata, like options used at compilation
	const CompileSettings*			Settings;

	/// Attributes that the shader expects
	const vector<ShaderAttributeDesc>	Attributes;

	const shared_ptr<ShaderSource>	Source;
};


/// General shader sampler
struct Sampler
{
public:
	Sampler(SamplerId Handle, const LocalDesc* Desc);

	/// Graphics API sampler handle.
	const SamplerId				Handle;		

	/// Sampler metadata gathered by compiling the source
	const LocalDesc*			Desc;
};


/// General shader uniform, global or local
struct Uniform
{
public:
	Uniform(NodeType Type, UniformId Handle, int ByteOffset, bool IsLocal);

	/// Type of the uniform variable. (float, vec2...)
	const NodeType				Type;
	
	/// Graphics API uniform handle. Only valid for uniforms in global space.
	/// For uniforms in a real uniform buffer (future feature) this should be -1.
	const UniformId				Handle;
	
	/// Offset in the uniform block/buffer.
	const int					ByteOffset;

	/// Tells whether it's a local uniform
	const bool					IsLocal;
};


/// Local uniform with manually controlled value (like a slot)
struct LocalUniform: public Uniform
{
	LocalUniform(NodeType Type, UniformId Handle, int ByteOffset, const LocalDesc* Desc);

	/// Metadata for this uniform, like UI name, etc.
	const LocalDesc*			Desc;
};


/// Shader globals parameter with automatically assigned value, like 
/// "uniform mat4x4 gViewSpace;"
struct GlobalUniform: public Uniform
{
	/// Possible usages of global uniforms
	enum UsageEnum
	{
		#undef ITEM
		#define ITEM(name, type, variable) name,
		GLOBALUSAGE_LIST
	};

	GlobalUniform(UsageEnum Usage, UniformId Handle, int ByteOffset);

	/// Uniform usage, like "ViewSpace matrix"
	const UsageEnum				Usage;				
};


/// Uniform block. Represents a group of uniforms in the shader.
/// Gets its values from a UniformArray (or UniformBuffer, not yet implemented).
class UniformBlock
{
public:
	UniformBlock(const vector<LocalUniform*>& Locals, 
		const vector<GlobalUniform*>& Globals, 
		const vector<Uniform*>& Uniforms, UINT ByteSize);

	/// List of local uniforms
	const vector<LocalUniform*>		Locals;

	/// List of global uniforms
	const vector<GlobalUniform*>	Globals;

	/// List of all uniforms, the two above
	const vector<Uniform*>			Uniforms;

	/// Size of the uniform array
	const UINT						ByteSize;
};

struct Globals
{
	#undef ITEM
	#define ITEM(name, type, token) type##_TYPE token;
	GLOBALUSAGE_LIST
};

extern Globals TheGlobals;

