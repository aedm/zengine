#pragma once

#include "../base/defines.h"
#include "../resources/texture.h"
#include "../shaders/valuetype.h"
#include <vector>
#include <string>

class OpenGLAPI;
extern OpenGLAPI* OpenGL;
extern bool GLDisableErrorChecks;

const int ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT = 4;

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
    Uniform(std::string name, ValueType type, UINT offset);

    /// Uniform name, must match the generated name inside the uniform block
    const std::string mName;

    /// Type (float, vec2...)
    const ValueType mType;

    /// Data offset inside the uniform block
    const UINT mOffset;
  };

  /// Sampler properties returned by the driver
  struct Sampler {
    Sampler(std::string name, SamplerId handle);

    /// Uniform name, must match the generated sampler name
    const std::string mName;

    /// Sampler ID
    const SamplerId mHandle;
  };

  struct SSBO {
    SSBO(std::string name, UINT index);
    const std::string mName;
    const UINT mIndex;
  };

  ShaderProgram(ShaderHandle shaderHandle, ShaderHandle vertexProgramHandle,
    ShaderHandle fragmentProgramHandle, std::vector<Uniform>& uniforms,
    std::vector<Sampler>& samplers, std::vector<SSBO>& ssbos,
    UINT uniformBlockSize);
  ~ShaderProgram();

  const ShaderHandle mProgramHandle;
  const ShaderHandle mVertexShaderHandle;
  const ShaderHandle mFragmentShaderHandle;
  const std::vector<Uniform> mUniforms;
  const std::vector<Sampler> mSamplers;
  const std::vector<SSBO> mSSBOs;
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
  bool IsEmpty() const;

  /// Uploads data to the buffer
  void UploadData(const void* data, int byteSize);
  DrawingAPIHandle GetHandle() const;
  int GetByteSize() const;

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
  std::shared_ptr<ShaderProgram> CreateShaderFromSource(const char* vertexSource,
    const char* fragmentSource);
  static void SetShaderProgram(const std::shared_ptr<ShaderProgram>& program, 
    const std::shared_ptr<Buffer>& uniformBuffer);
  static void EnableVertexAttribute(UINT index, ValueType nodeType, UINT offset, 
    UINT stride);

  /// Buffer functions
  void SetVertexBuffer(const std::shared_ptr<Buffer>& buffer);
  void SetIndexBuffer(const std::shared_ptr<Buffer>& buffer);
  static void SetSsbo(UINT index, const std::shared_ptr<Buffer>& buffer);

  void Render(const std::shared_ptr<Buffer>& indexBuffer,
    UINT Count, PrimitiveTypeEnum primitiveType,
    UINT instanceCount);

  /// Texture and surface handling
  static UINT GetTexelByteCount(TexelType type);

  std::shared_ptr<Texture> MakeTexture(int width, int height, TexelType type,
    const void* texelData, bool gpuMemoryOnly,
    bool isMultisample, bool doesRepeat, bool generateMipmaps);
  static void DeleteTextureGpuData(Texture::Handle handle);
  void UploadTextureGpuData(const std::shared_ptr<Texture>& texture, void* texelData);

  void SetTexture(const ShaderProgram::Sampler& sampler, 
    const std::shared_ptr<Texture>& texture, UINT slotIndex);

  /// Framebuffer operations
  static FrameBufferId CreateFrameBuffer(const std::shared_ptr<Texture>& depthBuffer,
    const std::shared_ptr<Texture>& targetBufferA,
    const std::shared_ptr<Texture>& targetBufferB);
  static void DeleteFrameBuffer(FrameBufferId frameBufferId);
  void SetFrameBuffer(FrameBufferId frameBufferId);
  static void BlitFrameBuffer(FrameBufferId source, FrameBufferId target,
    int srcX0, int srcY0, int srcX1, int srcY1,
    int dstX0, int dstY0, int dstX1, int dstY1);

  /// Render parameters
  static void SetViewport(int x, int y, int width, int height, float depthMin = 0.0f,
    float depthMax = 1.0f);
  void SetRenderState(const RenderState* State);

  /// Drawing
  void Clear(bool colorBuffer = true, bool depthBuffer = true, UINT rgbColor = 0);

private:
  static void SetTextureData(UINT width, UINT height, TexelType type, const void* texelData,
    bool generateMipmap);
  static void SetTextureSubData(UINT x, UINT y, UINT width, UINT height, TexelType type, 
    void* texelData);

  /// Shadowed buffer binds
  void BindVertexBuffer(VertexBufferHandle bufferId);
  void BindIndexBuffer(IndexBufferHandle bufferId);
  void BindTexture(Texture::Handle textureId);
  void BindMultisampleTexture(Texture::Handle textureId);
  void BindFrameBuffer(FrameBufferId frameBufferId);

  void SetActiveTexture(UINT activeTextureIndex); // silly OpenGL.

  void SetDepthTest(bool enable);
  void SetFaceMode(RenderState::FaceMode faceMode);
  void SetClearColor(UINT clearColor);

  void SetBlending(bool Enable);
  void SetBlendMode(RenderState::BlendMode blendMode);

  /// Shadow values
  FrameBufferId mBoundFrameBufferShadow{};
  VertexBufferHandle mBoundVertexBufferShadow{};
  IndexBufferHandle mBoundIndexBufferShadow{};

  Texture::Handle mBoundTextureShadow[MAX_COMBINED_TEXTURE_SLOTS]{};
  Texture::Handle mBoundMultisampleTextureShadow[MAX_COMBINED_TEXTURE_SLOTS]{};
  Texture::Handle mActiveTextureShadow{};

  bool mDepthTestEnabledShadow{};
  RenderState::FaceMode mFaceMode = RenderState::FaceMode::BACK;
  RenderState::BlendMode mBlendMode = RenderState::BlendMode::NORMAL;
  bool mBlendEnabled{};
};
