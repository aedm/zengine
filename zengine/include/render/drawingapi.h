#pragma once

#include "../base/defines.h"
#include "../resources/texture.h"
#include "../shaders/valuetype.h"
#include <vector>
#include <string>

using namespace std;

class OpenGLAPI;
extern OpenGLAPI* OpenGL;

const int ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT = 1;

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


/// A compiled shader stage
struct ShaderCompiledStage {
  /// Uniform properties returned by the driver
  struct Uniform {
    Uniform(const string& name, ValueType type, UINT offset);

    /// Uniform name, must match the generated name inside the uniform block
    const string mName;

    /// Type (float, vec2...)
    const ValueType mType;

    /// Data offset inside the uniform block
    const UINT mOffset;
  };

  /// Sampler properties returned by the driver
  struct Sampler {
    Sampler(const string& name, SamplerId handle);

    /// Uniform name, must match the generated sampler name
    const string mName;

    /// Sampler ID
    const SamplerId mHandle;
  };

  ShaderCompiledStage(ShaderHandle shaderObjectHandle,
    vector<Uniform>& uniforms, vector<Sampler>& samplers,
    UINT uniformBlockSize, ShaderHandle uniformBufferHandle, UINT uniformBinding);
  ~ShaderCompiledStage();

  const ShaderHandle mShaderObjectHandle;
  const ShaderHandle mUniformBufferHandle;
  const vector<Uniform> mUniforms;
  const vector<Sampler> mSamplers;
  const UINT mUniformBlockSize;
  const UINT mUniformBinding;
};


/// Output of platform-dependent shader compilation
/// All metadata is determined by the shader compiler of the OpenGL driver.
struct ShaderProgram {
  ShaderProgram(ShaderHandle shaderHandle, 
    const shared_ptr<ShaderCompiledStage>& vertexStage,
    const shared_ptr<ShaderCompiledStage>& fragmentStage);
  ~ShaderProgram();

  const ShaderHandle mProgramHandle;
  const shared_ptr<ShaderCompiledStage> mVertexStage;
  const shared_ptr<ShaderCompiledStage> mFragmentStage;
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

  /// Workaround for an nVidia driver bug. For some reason framebuffers
  /// cannot be bound after shader compilation before SwapBuffers. 
  /// This members marks whether shader compilation was done.
  bool mProgramCompiledHack = false;

  /// Shader functions
  shared_ptr<ShaderProgram> CreateShaderFromSource(const char* VertexSource,
    const char* FragmentSource);
  void SetShaderProgram(const shared_ptr<ShaderProgram>& program, 
    void* vertexUniforms, void* fragmentUniforms);

  /// Vertex buffer handling
  VertexBufferHandle CreateVertexBuffer(UINT Size);
  void DestroyVertexBuffer(VertexBufferHandle Handle);
  void* MapVertexBuffer(VertexBufferHandle Handle);
  void UnMapVertexBuffer(VertexBufferHandle Handle);
  void SetVertexBuffer(VertexBufferHandle Handle);
  void EnableVertexAttribute(UINT Index, ValueType Type, UINT Offset, UINT Stride);

  /// Index buffer handling
  IndexBufferHandle CreateIndexBuffer(UINT IndexCount);
  void DestroyIndexBuffer(IndexBufferHandle Handle);
  void* MapIndexBuffer(IndexBufferHandle Handle);
  void UnMapIndexBuffer(IndexBufferHandle Handle);
  void SetIndexBuffer(IndexBufferHandle Handle);

  void Render(IndexBufferHandle IndexBuffer,
    UINT Count,
    PrimitiveTypeEnum PrimitiveType,
    UINT InstanceCount);

  /// Texture and surface handling
  static UINT GetTexelByteCount(TexelType type);

  shared_ptr<Texture> MakeTexture(int width, int height, TexelType type,
    const shared_ptr<vector<char>>& texelData, bool gpuMemoryOnly,
    bool isMultisample, bool doesRepeat, bool generateMipmaps);
  void DeleteTextureGPUData(Texture::Handle handle);
  void UploadTextureGPUData(const shared_ptr<Texture>& texture, void* texelData);

  void SetTexture(const ShaderCompiledStage::Sampler& sampler,
    const shared_ptr<Texture>& texture, UINT slotIndex);

  /// Framebuffer operations
  FrameBufferId CreateFrameBuffer(const shared_ptr<Texture>& depthBuffer,
    const shared_ptr<Texture>& targetBufferA,
    const shared_ptr<Texture>& targetBufferB);
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
  void BindTexture(Texture::Handle TextureID);
  void BindMultisampleTexture(Texture::Handle TextureID);
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

  Texture::Handle BoundTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
  Texture::Handle BoundMultisampleTextureShadow[MAX_COMBINED_TEXTURE_SLOTS];
  Texture::Handle ActiveTextureShadow;

  bool DepthTestEnabledShadow;
  RenderState::FaceMode mFaceMode;
  RenderState::BlendMode mBlendMode;
  bool mBlendEnabled;
  UINT ClearColorShadow;

  RenderState* DefaultRenderState;
};
