#pragma once

#include "../base/defines.h"
#include "../dom/nodetype.h"
#include <vector>
#include <string>

using namespace std;

class OpenGLAPI;
extern OpenGLAPI* OpenGL;

const int ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT = 8;

/// TODO: query OpenGL for this value
static const int MAX_COMBINED_TEXTURE_SLOTS = 48; 

/// All states of the rendering pipeline
struct RenderState {
  enum class FaceMode {
    FRONT,
    BACK,
    FRONT_AND_BACK
  };

  enum class BlendMode {
    NORMAL,
    ADDITIVE,
    ALPHA,
  };

  bool mDepthTest;
  FaceMode mFaceMode;
  BlendMode mBlendMode;
};


/// Abstract class for mapping vertex buffers of a certain format to shader attributes
class AttributeMapper {
public:
  ~AttributeMapper() {}

protected:
  AttributeMapper() {}
};


/// One attribute as vertex buffer element
struct VertexAttribute {
  VertexAttributeUsage Usage;
  int Size;
  int Offset;
};


/// One attribute as shader input
struct ShaderAttributeDesc {
  NodeType Type;
  AttributeId Handle;
  VertexAttributeUsage Usage;
  string Name;
};


/// Uniform properties in a compiled shader
struct ShaderUniformDesc {
  /// Type (float, vec2...)
  NodeType UniformType;

  /// Uniform ID
  UniformId Handle;

  string Name;
};


/// Sampler properties in a compiled shader
struct ShaderSamplerDesc {
  /// Sampler ID
  SamplerId Handle;

  string Name;
};


/// Output of platform-dependent shader compilation
struct ShaderCompileDesc {
  ShaderHandle Handle;
  vector<ShaderUniformDesc> Uniforms;
  vector<ShaderAttributeDesc> Attributes;
  vector<ShaderSamplerDesc> Samplers;
};


enum PrimitiveTypeEnum {
  PRIMITIVE_TRIANGLES,
  PRIMITIVE_LINES,
  PRIMITIVE_LINE_STRIP,
};


class OpenGLAPI {
public:
  OpenGLAPI();
  ~OpenGLAPI();

  /// Resets renderer. Call this upon context switch.
  void OnContextSwitch();

  /// Shader functions
  OWNERSHIP ShaderCompileDesc* CreateShaderFromSource(const char* VertexSource, 
                                                      const char* FragmentSource);
  void SetShaderProgram(ShaderHandle Handle);
  void SetUniform(UniformId Id, NodeType Type, const void* Values);
  void DestroyShaderProgram(ShaderHandle Handle);

  /// Vertex buffer handling
  VertexBufferHandle CreateVertexBuffer(UINT Size);
  void DestroyVertexBuffer(VertexBufferHandle Handle);
  void* MapVertexBuffer(VertexBufferHandle Handle);
  void UnMapVertexBuffer(VertexBufferHandle Handle);
  AttributeMapper* CreateAttributeMapper(const vector<VertexAttribute>& SourceAttribs, 
                                         const vector<ShaderAttributeDesc>& ShaderAttribs, 
                                         UINT Stride);
  void SetVertexBuffer(VertexBufferHandle Handle);
  void EnableVertexAttribute(UINT Index, NodeType Type, UINT Offset, UINT Stride);

  /// Index buffer handling
  IndexBufferHandle CreateIndexBuffer(UINT IndexCount);
  void DestroyIndexBuffer(IndexBufferHandle Handle);
  void* MapIndexBuffer(IndexBufferHandle Handle);
  void UnMapIndexBuffer(IndexBufferHandle Handle);
  void SetIndexBuffer(IndexBufferHandle Handle);

  /// Rendering
  void RenderIndexedMesh(IndexBufferHandle IndexHandle,
                         UINT IndexCount,
                         VertexBufferHandle VertexHandle,
                         const AttributeMapper* Mapper,
                         PrimitiveTypeEnum PrimitiveType);

  void RenderMesh(VertexBufferHandle VertexHandle,
                  UINT VertexCount,
                  const AttributeMapper* Mapper,
                  PrimitiveTypeEnum PrimitiveType);

  void Render(IndexBufferHandle IndexBuffer,
              UINT Count,
              PrimitiveTypeEnum PrimitiveType,
              UINT InstanceCount);

  /// Texture and surface handling
  TextureHandle CreateTexture(int Width, int Height, TexelType Type, bool isMultiSample, 
                              bool doesRepeat, bool mipmap);
  void DeleteTexture(TextureHandle Handle);
  void UploadTextureData(TextureHandle Handle, int Width, int Height, TexelType Type, 
                         void* TexelData);
  void UploadTextureSubData(TextureHandle Handle, UINT X, UINT Y, int Width, int Height, 
                            TexelType Type, void* TexelData);
  void SetTexture(SamplerId Sampler, TextureHandle Texture, UINT SlotIndex, bool isMultiSample);

  /// Framebuffer operations
  FrameBufferId CreateFrameBuffer(TextureHandle depthBuffer,
                                  TextureHandle targetBufferA,
                                  TextureHandle targetBufferB,
                                  bool isMultiSample);
  void DeleteFrameBuffer(FrameBufferId frameBufferId);
  void SetFrameBuffer(FrameBufferId frameBufferid);
  void BlitFrameBuffer(FrameBufferId source, FrameBufferId target, 
                       int srcX0, int srcY0, int srcX1, int srcY1,
                       int dstX0, int dstY0, int dstX1, int dstY1);

  /// Render parameters
  void SetViewport(int X, int Y, int Width, int Height, float DepthMin = 0.0f, 
                   float DepthMax = 1.0f);
  void SetRenderState(const RenderState* State);

  /// Drawing
  void Clear(bool ColorBuffer = true, bool DepthBuffer = true, UINT RGBColor = 0);

private:
  void SetTextureData(UINT Width, UINT Height, TexelType Type, void* TexelData, 
                      bool generateMipmap);
  void SetTextureSubData(UINT X, UINT Y, UINT Width, UINT Height, TexelType Type, void* TexelData);

  /// Shadowed buffer binds
  void BindVertexBuffer(VertexBufferHandle BufferID);
  void BindIndexBuffer(IndexBufferHandle BufferID);
  void BindTexture(TextureHandle TextureID);
  void BindMultisampleTexture(TextureHandle TextureID);
  void BindFrameBuffer(FrameBufferId frameBufferID);

  void SetActiveTexture(UINT ActiveTextureIndex); // silly OpenGL.

  void SetDepthTest(bool Enable);
  void SetFaceMode(RenderState::FaceMode faceMode);
  void SetClearColor(UINT ClearColor);

  void SetBlending(bool Enable);
  void SetBlendMode(RenderState::BlendMode blendMode);

  /// Same as glNamedFramebufferXXXBuffer in OpenGL 4.5, but Intel doesn't fckn support that
  void BindReadFramebuffer(FrameBufferId framebuffer);
  void BindDrawFramebuffer(FrameBufferId framebuffer);
  void NamedFramebufferReadBuffer(FrameBufferId framebuffer, UINT mode);
  void NamedFramebufferDrawBuffer(FrameBufferId framebuffer, UINT mode);

  /// Shadow values
  FrameBufferId BoundFrameBufferReadBuffer;
  FrameBufferId BoundFrameBufferDrawBuffer;
  FrameBufferId BoundFrameBufferShadow;
  
  VertexBufferHandle BoundVertexBufferShadow;
  IndexBufferHandle BoundIndexBufferShadow;
  
  TextureHandle BoundTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
  TextureHandle BoundMultisampleTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
  TextureHandle ActiveTextureShadow;

  bool DepthTestEnabledShadow;
  RenderState::FaceMode mFaceMode;
  RenderState::BlendMode mBlendMode;
  bool mBlendEnabled;
  UINT ClearColorShadow;

  RenderState* DefaultRenderState;
};
