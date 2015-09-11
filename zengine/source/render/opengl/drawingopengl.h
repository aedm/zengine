#pragma once

#include <include/render/drawingapi.h>
#include <include/base/defines.h>

#define GLEW_STATIC
#include <glew/glew.h>

static const int MAX_COMBINED_TEXTURE_SLOTS = 48; /// TODO: query OpenGL for this value

struct MappedAttributeOpenGL
{
	GLuint						Index;
	GLint						Size;
	GLenum						Type;
	UINT						Offset;
};

class AttributeMapperOpenGL: public AttributeMapper
{
public:
	AttributeMapperOpenGL();
	virtual ~AttributeMapperOpenGL() {}

	void						Set() const;

	GLsizei						Stride;
	
	vector<MappedAttributeOpenGL>	MappedAttributes;
};


class DrawingOpenGL: public DrawingAPI
{
public:
	DrawingOpenGL();
	virtual ~DrawingOpenGL();

	/// Resets renderer. Call this upon context switch.
	virtual void				OnContextSwitch() override;

	/// Shader functions
	virtual OWNERSHIP ShaderCompileDesc* CreateShaderFromSource(const char* VertexSource, const char* FragmentSource) override;
	virtual void				SetShaderProgram(ShaderHandle Handle) override;
	virtual void				SetUniform(UniformId Id, NodeType Type, const void* Values) override;
	virtual void				DestroyShaderProgram(ShaderHandle Handle) override;

	/// Vertex buffer handling
	virtual VertexBufferHandle	CreateVertexBuffer(UINT Size) override;
	virtual void				DestroyVertexBuffer(VertexBufferHandle Handle) override;
	virtual void*				MapVertexBuffer(VertexBufferHandle Handle) override;
	virtual void				UnMapVertexBuffer(VertexBufferHandle Handle) override;
	virtual AttributeMapper*	CreateAttributeMapper(const vector<VertexAttribute>& SourceAttribs, const vector<ShaderAttributeDesc>& ShaderAttribs, UINT Stride) override;
	virtual void				SetVertexBuffer(VertexBufferHandle Handle) override;
	virtual void				EnableVertexAttribute(UINT Index, NodeType Type, UINT Offset, UINT Stride) override;

	/// Index buffer handling
	virtual IndexBufferHandle	CreateIndexBuffer(UINT IndexCount) override;
	virtual void				DestroyIndexBuffer(IndexBufferHandle Handle) override;
	virtual void*				MapIndexBuffer(IndexBufferHandle Handle) override;
	virtual void				UnMapIndexBuffer(IndexBufferHandle Handle) override;
	virtual void				SetIndexBuffer(IndexBufferHandle Handle) override;

	/// Texture and surface handling
	virtual TextureHandle		CreateTexture(int Width, int Height, TexelTypeEnum Type) override;
	virtual void				DeleteTexture(TextureHandle Handle) override;
	virtual void				UploadTextureData(TextureHandle Handle, int Width, int Height, TexelTypeEnum Type, void* TexelData) override;
	virtual void				UploadTextureSubData(TextureHandle Handle, UINT X, UINT Y, int Width, int Height, TexelTypeEnum Type, void* TexelData) override;
	virtual void				SetTexture(SamplerId Sampler, TextureHandle Texture, UINT SlotIndex) override;

	/// Render parameters
	virtual void				SetViewport(int X, int Y, int Width, int Height, float DepthMin, float DepthMax) override;
	virtual void				SetRenderState(const RenderState* State);

	/// Rendering
	virtual void				Clear(bool ColorBuffer, bool DepthBuffer, UINT RGBColor) override;
	virtual void				RenderIndexedMesh(IndexBufferHandle IndexHandle, 
									UINT IndexCount, 
									VertexBufferHandle VertexHandle, 
									const AttributeMapper* Mapper,
									PrimitiveTypeEnum PrimitiveType) override;
	virtual void				RenderMesh(VertexBufferHandle VertexHandle, 
									UINT VertexCount, 
									const AttributeMapper* Mapper,
									PrimitiveTypeEnum PrimitiveType) override;
	virtual void				Render(IndexBufferHandle IndexBuffer,
									UINT VertexCount,
									PrimitiveTypeEnum PrimitiveType,
                  UINT InstanceCount) override;

private:

	void						SetTextureData(UINT Width, UINT Height, TexelTypeEnum Type, void* TexelData);
	void						SetTextureSubData(UINT X, UINT Y, UINT Width, UINT Height, TexelTypeEnum Type, void* TexelData);

	/// Shadowed buffer binds
	void						BindVertexBuffer(GLuint BufferID);	
	void						BindIndexBuffer(GLuint BufferID);
	void						BindTexture(GLuint TextureID);
	void						SetActiveTexture(GLuint ActiveTextureIndex); // silly OpenGL.

	void						SetDepthTest(bool Enable);
	void						SetFace(RenderState::FaceEnum Face);
	void						SetClearColor(UINT ClearColor);

	void						SetBlending(bool Enable);
	void						SetBlendMode( RenderState::BlendModeEnum BlendMode );

	/// Shadow values
	GLuint						BoundVertexBufferShadow;
	GLuint						BoundIndexBufferShadow;
	GLuint						BoundTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
	GLuint						ActiveTextureShadow;						

	bool						DepthTestEnabledShadow;
	RenderState::FaceEnum		FaceShadow;
	RenderState::BlendModeEnum	BlendModeShadow;
	bool						BlendEnableShadow;
	UINT						ClearColorShadow;

	RenderState*				DefaultRenderState;
};
