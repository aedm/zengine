#pragma once

#include "../base/defines.h"
#include "../resources/texture.h"
#include "../shaders/valuetype.h"
#include <vector>
#include <string>

using namespace std;

class OpenGLAPI;
extern OpenGLAPI* OpenGL;
extern bool GLDisableErrorChecks;

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


/// Output of platform-dependent shader compilation
/// All metadata is determined by the shader compiler of the OpenGL driver.
struct ShaderProgram {
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

  struct SSBO {
    SSBO(const string& name, UINT index);
    const string mName;
    const UINT mIndex;
  };

  ShaderProgram(ShaderHandle shaderHandle, ShaderHandle vertexProgramHandle,
    ShaderHandle fragmentProgramHandle, vector<Uniform>& uniforms,
    vector<Sampler>& samplers, vector<SSBO>& ssbos,
    UINT uniformBlockSize);
  ~ShaderProgram();

  const ShaderHandle mProgramHandle;
  const ShaderHandle mVertexShaderHandle;
  const ShaderHandle mFragmentShaderHandle;
  const vector<Uniform> mUniforms;
  const vector<Sampler> mSamplers;
  const vector<SSBO> mSSBOs;
  const UINT mUniformBlockSize;
};

/// OpenGL general buffer object
class Buffer {
public:
  /// Creates a new buffer with a specific size. -1 means no resource allocation.
  Buffer(int byteSize = -1);
  ~Buffer();
  void Allocate(int byteSize);
  
  /// Returns true if there's no data in the buffer
  bool IsEmpty();

  /// Uploads data to the buffer
  void UploadData(const void* data, int byteSize);
  DrawingAPIHandle GetHandle();
  int GetByteSize();

private:
  void Release();

  DrawingAPIHandle mHandle = 0;
  int mByteSize = -1;
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
    const shared_ptr<Buffer>& uniformBuffer);
  void SetUniform(UniformId Id, ValueType Type, const void* Values);

  /// Vertex buffer handling
  //VertexBufferHandle CreateVertexBuffer(UINT Size);
  //void DestroyVertexBuffer(VertexBufferHandle Handle);
  //void* MapVertexBuffer(VertexBufferHandle Handle);
  //void UnMapVertexBuffer(VertexBufferHandle Handle);
  void SetVertexBuffer(const shared_ptr<Buffer>& buffer);
  void EnableVertexAttribute(UINT Index, ValueType Type, UINT Offset, UINT Stride);

  /// Index buffer handling
  //IndexBufferHandle CreateIndexBuffer(UINT IndexCount);
  //void DestroyIndexBuffer(IndexBufferHandle Handle);
  //void* MapIndexBuffer(IndexBufferHandle Handle);
  //void UnMapIndexBuffer(IndexBufferHandle Handle);
  void SetIndexBuffer(const shared_ptr<Buffer>& buffer);

  void Render(const shared_ptr<Buffer>& indexBuffer,
    UINT Count, PrimitiveTypeEnum primitiveType,
    UINT instanceCount);

  /// Texture and surface handling
  static UINT GetTexelByteCount(TexelType type);

  shared_ptr<Texture> MakeTexture(int width, int height, TexelType type,
    const shared_ptr<vector<char>>& texelData, bool gpuMemoryOnly,
    bool isMultisample, bool doesRepeat, bool generateMipmaps);
  void DeleteTextureGPUData(Texture::Handle handle);
  void UploadTextureGPUData(const shared_ptr<Texture>& texture, void* texelData);

  void SetTexture(const ShaderProgram::Sampler& sampler, 
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
