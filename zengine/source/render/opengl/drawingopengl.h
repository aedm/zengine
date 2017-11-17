//#pragma once
//
//#include <include/render/drawingapi.h>
//#include <include/base/defines.h>
//
//#define GLEW_STATIC
//#include <glew/glew.h>
//
//static const int MAX_COMBINED_TEXTURE_SLOTS = 48; /// TODO: query OpenGL for this value
//
//struct MappedAttributeOpenGL {
//  GLuint						Index;
//  GLint						Size;
//  GLenum						Type;
//  UINT						Offset;
//};
//
//class AttributeMapperOpenGL: public AttributeMapper {
//public:
//  AttributeMapperOpenGL();
//  virtual ~AttributeMapperOpenGL() {}
//
//  void						Set() const;
//
//  GLsizei						Stride;
//
//  vector<MappedAttributeOpenGL>	MappedAttributes;
//};
//
//
//class DrawingOpenGL: public DrawingAPI {
//public:
//  DrawingOpenGL();
//  virtual ~DrawingOpenGL();
//
//  /// Resets renderer. Call this upon context switch.
//  virtual void				OnContextSwitch() override;
//
//  /// Shader functions
//  virtual OWNERSHIP ShaderCompileDesc* CreateShaderFromSource(const char* VertexSource, const char* FragmentSource) override;
//  virtual void				SetShaderProgram(ShaderHandle Handle) override;
//  virtual void				SetUniform(UniformId Id, NodeType Type, const void* Values) override;
//  virtual void				DestroyShaderProgram(ShaderHandle Handle) override;
//
//  /// Vertex buffer handling
//  virtual VertexBufferHandle	CreateVertexBuffer(UINT Size) override;
//  virtual void				DestroyVertexBuffer(VertexBufferHandle Handle) override;
//  virtual void*				MapVertexBuffer(VertexBufferHandle Handle) override;
//  virtual void				UnMapVertexBuffer(VertexBufferHandle Handle) override;
//  virtual AttributeMapper*	CreateAttributeMapper(const vector<VertexAttribute>& SourceAttribs, 
//                                                  const vector<ShaderProgram::Attribute>& ShaderAttribs, UINT Stride) override;
//  virtual void				SetVertexBuffer(VertexBufferHandle Handle) override;
//  virtual void				EnableVertexAttribute(UINT Index, NodeType Type, UINT Offset, UINT Stride) override;
//
//  /// Index buffer handling
//  virtual IndexBufferHandle	CreateIndexBuffer(UINT IndexCount) override;
//  virtual void				DestroyIndexBuffer(IndexBufferHandle Handle) override;
//  virtual void*				MapIndexBuffer(IndexBufferHandle Handle) override;
//  virtual void				UnMapIndexBuffer(IndexBufferHandle Handle) override;
//  virtual void				SetIndexBuffer(IndexBufferHandle Handle) override;
//
//  /// Texture and surface handling
//  virtual TextureHandle		CreateTexture(int Width, int Height, TexelType Type, bool isMultiSample, bool doesRepeat, bool mipmap) override;
//  virtual void				DeleteTexture(TextureHandle Handle) override;
//  virtual void				UploadTextureData(TextureHandle Handle, int Width, int Height, TexelType Type, void* TexelData) override;
//  virtual void				UploadTextureSubData(TextureHandle Handle, UINT X, UINT Y, int Width, int Height, TexelType Type, void* TexelData) override;
//  virtual void				SetTexture(SamplerId Sampler, TextureHandle Texture, UINT SlotIndex, bool isRenderTarget) override;
//
//  /// Framebuffer operations
//  virtual FrameBufferId CreateFrameBuffer(TextureHandle depthBuffer,
//                                          TextureHandle targetBufferA,
//                                          TextureHandle targetBufferB,
//                                          bool isMultiSample) override;
//  virtual void DeleteFrameBuffer(FrameBufferId frameBufferId) override;
//  virtual void SetFrameBuffer(FrameBufferId frameBufferid) override;
//  virtual void BlitFrameBuffer(FrameBufferId source, FrameBufferId target, int srcX0, int srcY0, int srcX1, int srcY1,
//                               int dstX0, int dstY0, int dstX1, int dstY1) override;
//
//  /// Render parameters
//  virtual void				SetViewport(int X, int Y, int Width, int Height, float DepthMin, float DepthMax) override;
//  virtual void				SetRenderState(const RenderState* State);
//
//  /// Rendering
//  virtual void				Clear(bool ColorBuffer, bool DepthBuffer, UINT RGBColor) override;
//  virtual void				RenderIndexedMesh(IndexBufferHandle IndexHandle,
//                                        UINT IndexCount,
//                                        VertexBufferHandle VertexHandle,
//                                        const AttributeMapper* Mapper,
//                                        PrimitiveTypeEnum PrimitiveType) override;
//  virtual void				RenderMesh(VertexBufferHandle VertexHandle,
//                                 UINT VertexCount,
//                                 const AttributeMapper* Mapper,
//                                 PrimitiveTypeEnum PrimitiveType) override;
//  virtual void				Render(IndexBufferHandle IndexBuffer,
//                             UINT VertexCount,
//                             PrimitiveTypeEnum PrimitiveType,
//                             UINT InstanceCount) override;
//
//private:
//
//  void						SetTextureData(UINT Width, UINT Height, TexelType Type, void* TexelData, bool generateMipmap);
//  void						SetTextureSubData(UINT X, UINT Y, UINT Width, UINT Height, TexelType Type, void* TexelData);
//
//  /// Shadowed buffer binds
//  void						BindVertexBuffer(GLuint BufferID);
//  void						BindIndexBuffer(GLuint BufferID);
//  void						BindTexture(GLuint TextureID);
//  void						BindMultisampleTexture(GLuint TextureID);
//  void						BindFrameBuffer(GLuint frameBufferID);
//
//  void						SetActiveTexture(GLuint ActiveTextureIndex); // silly OpenGL.
//
//  void						SetDepthTest(bool Enable);
//  void						SetFace(RenderState::FaceEnum Face);
//  void						SetClearColor(UINT ClearColor);
//
//  void						SetBlending(bool Enable);
//  void						SetBlendMode(RenderState::BlendModeEnum BlendMode);
//
//  /// Same as glNamedFramebufferXXXBuffer in OpenGL 4.5, but Intel doesn't fckn support that
//  void						BindReadFramebuffer(GLuint framebuffer);
//  void						BindDrawFramebuffer(GLuint framebuffer);
//  void						NamedFramebufferReadBuffer(GLuint framebuffer, GLenum mode);
//  void						NamedFramebufferDrawBuffer(GLuint framebuffer, GLenum mode);
//
//  /// Shadow values
//  GLuint						BoundFrameBufferReadBuffer;
//  GLuint						BoundFrameBufferDrawBuffer;
//  GLuint						BoundVertexBufferShadow;
//  GLuint						BoundIndexBufferShadow;
//  GLuint						BoundFrameBufferShadow;
//  GLuint						BoundTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
//  GLuint						BoundMultisampleTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
//  GLuint						ActiveTextureShadow;
//
//  bool						DepthTestEnabledShadow;
//  RenderState::FaceEnum		FaceShadow;
//  RenderState::BlendModeEnum	BlendModeShadow;
//  bool						BlendEnableShadow;
//  UINT						ClearColorShadow;
//
//  RenderState*				DefaultRenderState;
//};
