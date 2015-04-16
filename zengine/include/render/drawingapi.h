#pragma once

#include "../base/defines.h"
#include "../base/types.h"
#include <vector>
#include <string>

using namespace std;

class DrawingAPI;
class AttributeMapper;
struct ShaderCompileDesc;
struct ShaderUniformDesc;
struct ShaderAttributeDesc;
struct VertexAttribute;
struct RenderState;
//class Surface;
//class RenderTarget;
extern DrawingAPI* TheDrawingAPI;

/// Size of a vertex attribute in bytes
//extern const int* VertexAttributeSize;

enum PrimitiveTypeEnum {
	PRIMITIVE_TRIANGLES,
	PRIMITIVE_LINES,
	PRIMITIVE_LINE_STRIP,
};

class DrawingAPI
{
public:
	virtual ~DrawingAPI() {}

	/// Renderer setup
	//virtual void 				Init(void* Handle, bool Doublebuffer) = 0;
	//virtual OWNERSHIP Surface*	CreateSurface(void* Handle, int Width, int Height) = 0; 
	//virtual bool				ResizeSurface(Surface* DrawDev, int Width, int Height) = 0;

	/// Resets renderer. Call this upon context switch.
	virtual void				OnContextSwitch() = 0;

	/// Shader functions
	virtual OWNERSHIP ShaderCompileDesc* CreateShaderFromSource(const char* VertexSource, const char* FragmentSource) = 0;
	virtual void				SetShaderProgram(ShaderHandle Handle) = 0;
	virtual void				SetUniform(UniformId Id, NodeType Type, const void* Values) = 0;
	virtual void				DestroyShaderProgram(ShaderHandle Handle) = 0;

	/// Vertex buffer handling
	virtual VertexBufferHandle	CreateVertexBuffer(UINT Size) = 0;
	virtual void				DestroyVertexBuffer(VertexBufferHandle Handle) = 0;
	virtual void*				MapVertexBuffer(VertexBufferHandle Handle) = 0;
	virtual void				UnMapVertexBuffer(VertexBufferHandle Handle) = 0;
	virtual AttributeMapper*	CreateAttributeMapper(const vector<VertexAttribute>& SourceAttribs, const vector<ShaderAttributeDesc>& ShaderAttribs, UINT Stride) = 0;

	/// Index buffer handling
	virtual IndexBufferHandle	CreateIndexBuffer(UINT IndexCount) = 0;
	virtual void				DestroyIndexBuffer(IndexBufferHandle Handle) = 0;
	virtual void*				MapIndexBuffer(IndexBufferHandle Handle) = 0;
	virtual void				UnMapIndexBuffer(IndexBufferHandle Handle) = 0;

	/// Rendering
	virtual void				RenderIndexedMesh(IndexBufferHandle IndexHandle, 
									UINT IndexCount, 
									VertexBufferHandle VertexHandle, 
									const AttributeMapper* Mapper, 
									PrimitiveTypeEnum PrimitiveType) = 0;
	virtual void				RenderMesh(VertexBufferHandle VertexHandle, 
									UINT VertexCount, 
									const AttributeMapper* Mapper,
									PrimitiveTypeEnum PrimitiveType) = 0;
	//virtual void				RenderMesh(const Mesh* MeshInstance) = 0;
	//virtual void				RenderTrianglesDirect(int VertexCount, int PrimitiveCount, const VertexFormat* Format, const void* Vertices, const void* Indices) = 0;


	//virtual void				UploadIndices(IndexBufferHandle Handle, UINT IndexCount, IndexEntry* Indices) = 0;
	//virtual VertexDeclaration	CreateVertexDeclaration(const vector<VertexAttribute>& Vertex) = 0;
	//virtual void				DestroyVertexDeclaration(VertexDeclaration Declaration) = 0;
	//virtual void				RenderWireframeMesh(const Mesh* MeshInstance) = 0;
	//virtual void				RenderLinesDirect(int VertexCount, int PrimitiveCount, const VertexFormat* Format, const void* Vertices, const void* Indices) = 0;

	/// Texture and surface handling
	virtual TextureHandle		CreateTexture(int Width, int Height, TexelTypeEnum Type) = 0;
	virtual void				DeleteTexture(TextureHandle Handle) = 0;
	virtual void				UploadTextureData(TextureHandle Handle, int Width, int Height, TexelTypeEnum Type, void* TexelData) = 0;
	virtual void				UploadTextureSubData(TextureHandle Handle, UINT X, UINT Y, int Width, int Height, TexelTypeEnum Type, void* TexelData) = 0;
	virtual void				SetTexture(SamplerId Sampler, TextureHandle Texture, UINT SlotIndex) = 0;
	//virtual RenderTarget*		CreateRenderTarget() = 0;
	//virtual void				GrabTexture(UCHAR* Argb8TargetBuffer, UINT Span, Texture* Tex) = 0;
	//virtual void				GrabSurface(UCHAR* Argb8TargetBuffer, UINT Span, UINT TargetWidth, UINT TargetHeight, RenderTarget* RTarget) = 0;

	/// Render parameters
	virtual void				SetViewport(int X, int Y, int Width, int Height, float DepthMin = 0.0f, float DepthMax = 1.0f) = 0;
	virtual void				SetRenderState(const RenderState* State) = 0;

	/// Drawing
	virtual void				Clear(bool ColorBuffer = true, bool DepthBuffer = true, UINT RGBColor = 0) = 0;
	//virtual void				BeginRender() = 0;
	//virtual void				EndRender() = 0;

	//virtual void				RenderText(int X, int Y, int Width, int Height, const wchar_t* Text, UINT Color, Align Alignment = ALIGN_LEFT) = 0;

	
	/// Default rendering device
	//Surface*					DefaultSurface;
};

//class RenderTarget
//{
//public:
//	virtual ~RenderTarget() {}
//
//	virtual void 				SetTexture(Texture* TextureInstance) = 0;
//	virtual void 				SetDepthBuffer(TextureHandle DepthTexture) = 0;
//	virtual void 				Set() = 0;
//};

//class Surface
//{
//public:
//	virtual ~Surface() {}
//
//	virtual void				Present() = 0;
//
//	RenderTarget*				DefaultRenderTarget;
//};

/// All states of the rendering pipeline
struct RenderState
{
	enum FaceEnum {
		FACE_FRONT,
		FACE_BACK,
		FACE_FRONT_AND_BACK
	};

	enum BlendModeEnum {
		BLEND_NORMAL,
		BLEND_ADDITIVE,
		BLEND_ALPHA,
	};

	bool						DepthTest;
	FaceEnum					Face;
	BlendModeEnum				BlendMode;
};

/// Abstract class for mapping vertex buffers of a certain format to shader attributes
class AttributeMapper 
{
public:
	virtual ~AttributeMapper() {}

protected:
	AttributeMapper() {}
};

/// One attribute as vertex buffer element
struct VertexAttribute
{
	VertexAttributeEnum			Usage;
	int							Size;
	int							Offset;
};

/// One attribute as shader input
struct ShaderAttributeDesc
{
	NodeType					Type;
	AttributeId					Handle;
	VertexAttributeEnum			Usage;
	string						Name;
};

/// Uniform properties in a compiled shader
struct ShaderUniformDesc
{
	NodeType					UniformType;		/// Type (float, vec2...)
	UniformId					Handle;				/// Uniform ID
	string						Name;
};

/// Sampler properties in a compiled shader
struct ShaderSamplerDesc
{
	SamplerId					Handle;				/// Sampler ID
	string						Name;
};

/// Output of platform-dependent shader compilation
struct ShaderCompileDesc
{
	ShaderHandle				Handle;
	vector<ShaderUniformDesc>	Uniforms;
	vector<ShaderAttributeDesc>	Attributes;
	vector<ShaderSamplerDesc>	Samplers;
};
