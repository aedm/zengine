#include <include/render/drawingapi.h>
#include <include/base/helpers.h>
#include <Windows.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <utility>
#include <memory>

bool GLDisableErrorChecks = false;


#ifdef _DEBUG
void CheckGLError() {
  if (GLDisableErrorChecks) return;
  const GLenum error = glGetError();
  ASSERT(error == GL_NO_ERROR);
}
#else
#	define CheckGLError()
#endif

struct GLVersion { GLboolean* version; const wchar_t* name; };
static GLVersion gOpenGLVersions[] = {
  {&__GLEW_VERSION_1_1, L"1.1"},
  {&__GLEW_VERSION_1_1, L"1.1"},
  {&__GLEW_VERSION_1_2, L"1.2"},
  {&__GLEW_VERSION_1_2_1, L"1.2.1"},
  {&__GLEW_VERSION_1_3, L"1.3"},
  {&__GLEW_VERSION_1_4, L"1.4"},
  {&__GLEW_VERSION_1_5, L"1.5"},
  {&__GLEW_VERSION_2_0, L"2.0"},
  {&__GLEW_VERSION_2_1, L"2.1"},
  {&__GLEW_VERSION_3_0, L"3.0"},
  {&__GLEW_VERSION_3_1, L"3.1"},
  {&__GLEW_VERSION_3_2, L"3.2"},
  {&__GLEW_VERSION_3_3, L"3.3"},
  {&__GLEW_VERSION_4_0, L"4.0"},
  {&__GLEW_VERSION_4_1, L"4.1"},
  {&__GLEW_VERSION_4_2, L"4.2"},
  {&__GLEW_VERSION_4_3, L"4.3"},
  {&__GLEW_VERSION_4_4, L"4.4"},
  {&__GLEW_VERSION_4_5, L"4.5"},
  {nullptr, nullptr}
};

OpenGLAPI::OpenGLAPI() {
  const GLenum err = glewInit();
  ASSERT(err == GLEW_OK);
  if (err != GLEW_OK) {
    ERR(L"Cannot initialize OpenGL.");
    SHOULD_NOT_HAPPEN;
  }
  else {
    const wchar_t* versionName = nullptr;
    for (GLVersion* version = gOpenGLVersions; version->version != nullptr; version++) {
      if (*version->version) versionName = version->name;
    }

    if (versionName == nullptr) ERR(L"OpenGL not found at all.");
    else INFO(L"OpenGL version %s found.", versionName);

    if (!GLEW_VERSION_4_5) {
      ERR(L"Sorry, OpenGL 4.5 needed at least.");
    }
    CheckGLError();
  }

  //glEnable(GL_MULTISAMPLE);
  OnContextSwitch();
  CheckGLError();
}


OpenGLAPI::~OpenGLAPI() = default;


void OpenGLAPI::OnContextSwitch() {
  /// Set defaults (shadow values must be something different at the beginning 
  /// to avoid false cache hit)
  mFaceMode = RenderState::FaceMode::BACK;
  SetFaceMode(RenderState::FaceMode::FRONT_AND_BACK);

  mBlendMode = RenderState::BlendMode::ADDITIVE;
  mBlendEnabled = true;
  SetBlendMode(RenderState::BlendMode::NORMAL);

  mDepthTestEnabledShadow = true;
  SetDepthTest(false);
  SetClearColor(0);

  glDepthMask(true);

  mActiveTextureShadow = -1;
  mBoundVertexBufferShadow = -1;
  mBoundIndexBufferShadow = -1;
  mBoundFrameBufferShadow = -1;
  for (int i = 0; i < MAX_COMBINED_TEXTURE_SLOTS; i++) {
    mBoundTextureShadow[i] = GLuint(-1);
    mBoundMultisampleTextureShadow[i] = GLuint(-1);
  }
  CheckGLError();
}

static ShaderHandle CompileAndAttachShader(GLuint program, GLuint shaderType,
  const char* source) {
  ASSERT(!PleaseNoNewResources);

  CheckGLError();
  /// Create shader object, set the source, and compile
  const GLuint shader = glCreateShader(shaderType);
  GLint length = GLint(strlen(source));
  glShaderSource(shader, 1, static_cast<const char **>(&source), &length);
  glCompileShader(shader);

  /// Make sure the compilation was successful
  GLint result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
    INFO("\n%s", source);

    /// Get the shader info log
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    char *log = new char[length];
    glGetShaderInfoLog(shader, length, &result, log);
    ERR(log);
    glDeleteShader(shader);
    CheckGLError();
    return 0;
  }

  glAttachShader(program, shader);
  CheckGLError();
  return shader;
}

void CollectUniformsFromProgram(GLuint program,
                                std::vector<ShaderProgram::Uniform>& uniforms, 
  UINT* oBlockSize)
{
  /// Query uniform blocks
  GLint uniformBlockCount;
  glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES,
    &uniformBlockCount);
  /// There should only be "Uniforms"
  ASSERT(uniformBlockCount == 1);

  /// And it should be at index 0
  const int uniformBlockIndex = glGetUniformBlockIndex(program, "Uniforms");
  ASSERT(uniformBlockIndex == 0);

  /// This is how OpenGL interface API works. Beautiful like a megmikrózott kefir.
  /// Get the number of active uniforms inside the uniform block, and the block size.
  const GLenum blockPropsList[] = { GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE };
#pragma pack(push, 1)
  struct { GLint mUniformCount, mSize; } blockProps{};
#pragma pack(pop)
  glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, uniformBlockIndex,
    ElementCount(blockPropsList), blockPropsList,
    ElementCount(blockPropsList), nullptr,
    reinterpret_cast<GLint*>(&blockProps));

  uniforms.reserve(blockProps.mUniformCount);

  std::vector<GLint> uniformLocations(blockProps.mUniformCount);
  const GLenum activeUniformProp[1] = { GL_ACTIVE_VARIABLES };
  glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, uniformBlockIndex, 1, 
    activeUniformProp, blockProps.mUniformCount, nullptr, &uniformLocations[0]);

  /// Query the properties of each uniform inside the block
  const GLenum uniformProperties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_OFFSET };
#pragma pack(push, 1)
  struct { GLint mNameLength, mType, mLocation, mOffset; } values{};
#pragma pack(pop)

  for (int blockIndex = 0; blockIndex < blockProps.mUniformCount; ++blockIndex) {
    glGetProgramResourceiv(program, GL_UNIFORM, uniformLocations[blockIndex],
      ElementCount(uniformProperties), uniformProperties,
      ElementCount(uniformProperties), nullptr,
      reinterpret_cast<GLint*>(&values));

    /// Get the name
    std::vector<char> name(values.mNameLength);
    glGetProgramResourceName(program, GL_UNIFORM, uniformLocations[blockIndex],
      values.mNameLength, nullptr, &name[0]);

    ValueType nodeType;
    switch (values.mType) {
    case GL_FLOAT:		  nodeType = ValueType::FLOAT;		break;
    case GL_FLOAT_VEC2:	nodeType = ValueType::VEC2;		  break;
    case GL_FLOAT_VEC3:	nodeType = ValueType::VEC3;		  break;
    case GL_FLOAT_VEC4:	nodeType = ValueType::VEC4;		  break;
    case GL_FLOAT_MAT4:	nodeType = ValueType::MATRIX44;	break;
    default: SHOULD_NOT_HAPPEN; break;
    }

    uniforms.emplace_back(std::string(&name[0]), nodeType, values.mOffset);
  }
  *oBlockSize = blockProps.mSize;
}

void CollectSSBOsFromProgram(GLuint program, std::vector<ShaderProgram::SSBO>& ssbos) {
  /// Query buffer indexes
  GLint bufferCount, ssboCount;
  glGetProgramInterfaceiv(program, GL_SHADER_STORAGE_BLOCK, GL_ACTIVE_RESOURCES,
    &bufferCount);
  glGetProgramInterfaceiv(program, GL_BUFFER_VARIABLE, GL_ACTIVE_RESOURCES,
    &ssboCount);
  INFO("Buffer count: %d, SSBO count: %d", bufferCount, ssboCount);

  /// TODO: query name length instead
  char name[2048];
  for (int i = 0; i < ssboCount; i++) {
    const GLenum blockPropsList[] = { GL_REFERENCED_BY_VERTEX_SHADER,
      GL_REFERENCED_BY_FRAGMENT_SHADER };
    const int propCount = ElementCount(blockPropsList);
    GLint props[propCount];
    glGetProgramResourceiv(program, GL_SHADER_STORAGE_BLOCK, i, propCount,
      blockPropsList, propCount, nullptr, props);
    const GLint isReferenced = (props[0] + props[1]) > 0;
    glGetProgramResourceName(program, GL_SHADER_STORAGE_BLOCK, i, sizeof(name), nullptr,
      name);
    GLuint resouceIndex =
      glGetProgramResourceIndex(program, GL_SHADER_STORAGE_BLOCK, name);
    INFO("SSBO #%d name: %s, index: %d, used: %d",
      i, name, resouceIndex, isReferenced);

    if (isReferenced) {
      /// Set up binding point
      glShaderStorageBlockBinding(program, resouceIndex, resouceIndex);
      ssbos.emplace_back(std::string(name), resouceIndex);
    }
  }
}

void CollectOpaqueFromProgram(GLuint program, std::vector<ShaderProgram::Sampler>& samplers) {
  GLint uniformCount;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

  GLint uniformNameMaxLength;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameMaxLength);
  std::vector<char> samplerName(uniformNameMaxLength);

  for (int uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++) {
    /// Request info about Nth uniform
    GLint nameLength;
    GLsizei size;
    GLenum type;
    glGetActiveUniform(program, uniformIndex, uniformNameMaxLength, &nameLength, &size,
      &type, &samplerName[0]);
    if (type == GL_SAMPLER_2D || type == GL_SAMPLER_2D_MULTISAMPLE ||
      type == GL_SAMPLER_2D_SHADOW) {
      const GLint location = glGetUniformLocation(program, &samplerName[0]);
      samplers.emplace_back(std::string(&samplerName[0]), location);
    }
  }
  CheckGLError();
}

bool LinkProgram(GLuint program) {
  /// Link program
  glLinkProgram(program);

  GLint result, length;
  glGetProgramiv(program, GL_LINK_STATUS, &result);
  glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
  if (length > 1) {
    char *log = new char[length];
    int charCount;
    glGetProgramInfoLog(program, length, &charCount, log);
    if (result == GL_FALSE) {
      ERR("Can't link shader: %s", log);
    }
    else {
      WARN("Shader compiler: %s", log);
    }
    CheckGLError();
    delete[] log;
  }

  return result == GL_TRUE;
}

std::shared_ptr<ShaderProgram> OpenGLAPI::CreateShaderFromSource(
  const char* vertexSource, const char* fragmentSource) 
{
  ASSERT(!PleaseNoNewResources);
  CheckGLError();
  GLuint program = glCreateProgram();
  mProgramCompiledHack = true;

  CheckGLError();
  /// Compile shaders
  ShaderHandle vertexShaderHandle =
    CompileAndAttachShader(program, GL_VERTEX_SHADER, vertexSource);
  if (vertexShaderHandle == 0) {
    WARN(L"Vertex shader compilation failed.");
    glDeleteProgram(program);
    CheckGLError();
    return nullptr;
  }
  ShaderHandle fragmentShaderHandle =
    CompileAndAttachShader(program, GL_FRAGMENT_SHADER, fragmentSource);
  if (fragmentShaderHandle == 0) {
    WARN(L"Fragment shader compilation failed.");
    CheckGLError();
    glDeleteProgram(program);
    glDeleteShader(vertexShaderHandle);
    CheckGLError();
    return nullptr;
  }

  if (!LinkProgram(program)) {
    glDeleteProgram(program);
    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);
    CheckGLError();
    return nullptr;
  }

  UINT uniformBlockSize;
  std::vector<ShaderProgram::Uniform> uniforms;
  std::vector<ShaderProgram::Sampler> samplers;
  std::vector<ShaderProgram::SSBO> ssbos;
  CollectUniformsFromProgram(program, uniforms, &uniformBlockSize);
  CollectOpaqueFromProgram(program, samplers);
  CollectSSBOsFromProgram(program, ssbos);
  
  return std::make_shared<ShaderProgram>(program, vertexShaderHandle, fragmentShaderHandle,
    uniforms, samplers, ssbos, uniformBlockSize);
}


void OpenGLAPI::SetShaderProgram(const std::shared_ptr<ShaderProgram>& program,
  const std::shared_ptr<Buffer>& uniformBuffer) 
{
  CheckGLError();
  glUseProgram(program->mProgramHandle);
  CheckGLError();
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, uniformBuffer->GetHandle());
  CheckGLError();
}


void OpenGLAPI::BindVertexBuffer(VertexBufferHandle bufferId) {
  if (bufferId != mBoundVertexBufferShadow) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferId);
    CheckGLError();
    mBoundVertexBufferShadow = bufferId;
  }
}


void OpenGLAPI::BindIndexBuffer(IndexBufferHandle bufferId) {
  if (bufferId != mBoundIndexBufferShadow) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
    CheckGLError();
    mBoundIndexBufferShadow = bufferId;
  }
}


void OpenGLAPI::BindFrameBuffer(FrameBufferId frameBufferId) {
  if (frameBufferId != mBoundFrameBufferShadow) {
    CheckGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferId);
    CheckGLError();
    mBoundFrameBufferShadow = frameBufferId;
#ifdef _DEBUG
    if (frameBufferId != 0) {
      GLenum state = glCheckNamedFramebufferStatus(frameBufferId, GL_FRAMEBUFFER);
      ASSERT(state == GL_FRAMEBUFFER_COMPLETE);
    }
#endif
  }
}


GLenum GetGLPrimitive(PrimitiveTypeEnum primitiveType) {
  switch (primitiveType) {
  case PRIMITIVE_LINES:		return GL_LINES;
  case PRIMITIVE_LINE_STRIP:	return GL_LINE_STRIP;
  case PRIMITIVE_TRIANGLES:	return GL_TRIANGLES;
  }
  SHOULD_NOT_HAPPEN;
  return 0;
}


void OpenGLAPI::Render(const std::shared_ptr<Buffer>& indexBuffer, UINT count,
  PrimitiveTypeEnum primitiveType, UINT instanceCount) 
{
  CheckGLError();
  if (indexBuffer != nullptr && indexBuffer->GetHandle() > 0) {
    BindIndexBuffer(indexBuffer->GetHandle());
    CheckGLError();
    glDrawElementsInstanced(
      GetGLPrimitive(primitiveType), count, GL_UNSIGNED_INT, nullptr, instanceCount);
  }
  else {
    glDrawArraysInstanced(GetGLPrimitive(primitiveType), 0, count, instanceCount);
  }
  CheckGLError();
}


void OpenGLAPI::SetViewport(int x, int y, int width, int height,
  float depthMin /*= 0.0f*/, float depthMax /*= 1.0f*/) {
  glViewport(x, y, width, height);
  glDepthRange(depthMin, depthMax);
  CheckGLError();
}


void OpenGLAPI::Clear(bool colorBuffer, bool depthBuffer, UINT rgbColor /*= 0*/) {
  SetClearColor(rgbColor);
  CheckGLError();
  glClear((colorBuffer ? GL_COLOR_BUFFER_BIT : 0) |
    (depthBuffer ? GL_DEPTH_BUFFER_BIT : 0));
  CheckGLError();
}


void OpenGLAPI::SetRenderState(const RenderState* state) {
  SetDepthTest(state->mDepthTest);
  SetFaceMode(state->mFaceMode);
  SetBlendMode(state->mBlendMode);
  CheckGLError();
}


void OpenGLAPI::SetDepthTest(bool enable) {
  if (enable == mDepthTestEnabledShadow) return;
  if (enable) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  }
  else glDisable(GL_DEPTH_TEST);
  mDepthTestEnabledShadow = enable;
}


void OpenGLAPI::SetBlendMode(RenderState::BlendMode blendMode) {
  if (blendMode == mBlendMode) return;
  switch (blendMode) {
  case RenderState::BlendMode::NORMAL:
    SetBlending(false);
    break;
  case RenderState::BlendMode::ADDITIVE:
    SetBlending(true);
    glBlendFunc(GL_ONE, GL_ONE);
    break;
  case RenderState::BlendMode::ALPHA:
    SetBlending(true);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  }
  mBlendMode = blendMode;
  CheckGLError();
}


void OpenGLAPI::SetBlending(bool enable) {
  if (enable == mBlendEnabled) return;
  if (enable) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  mBlendEnabled = enable;
  CheckGLError();
}


void OpenGLAPI::SetFaceMode(RenderState::FaceMode faceMode) {
  if (mFaceMode == faceMode) return;
  switch (faceMode) {
  case RenderState::FaceMode::FRONT:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    break;
  case RenderState::FaceMode::FRONT_AND_BACK:
    glDisable(GL_CULL_FACE);
    break;
  case RenderState::FaceMode::BACK:
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    break;
  }
  mFaceMode = faceMode;
  CheckGLError();
}


inline float IntColorToFloat(UINT color) {
  return float(color) / 255.0f;
}


void OpenGLAPI::SetClearColor(UINT clearColor) {
  glClearColor(
    IntColorToFloat((clearColor >> 16) & 0xff),
    IntColorToFloat((clearColor >> 8) & 0xff),
    IntColorToFloat((clearColor) & 0xff),
    1.0f);
  CheckGLError();
}


/// Converts a single TexelType to OpenGL enums
static void GetTextureType(TexelType type, GLint &internalFormat, GLenum &format,
  GLenum &glType) {
  switch (type) {
  case TexelType::ARGB8:
    internalFormat = GL_RGBA;
    format = GL_BGRA;
    glType = GL_UNSIGNED_BYTE;
    break;
  case TexelType::ARGB16:
    internalFormat = GL_RGBA16;
    format = GL_BGRA;
    glType = GL_UNSIGNED_SHORT;
    break;
  case TexelType::ARGB16F:
    internalFormat = GL_RGBA16F;
    format = GL_BGRA;
    glType = GL_FLOAT;
    break;
  case TexelType::ARGB32F:
    internalFormat = GL_RGBA32F;
    format = GL_BGRA;
    glType = GL_FLOAT;
    break;
  case TexelType::DEPTH32F:
    internalFormat = GL_DEPTH_COMPONENT32F;
    format = GL_DEPTH_COMPONENT;
    glType = GL_FLOAT;
    break;
  default:
    NOT_IMPLEMENTED;
    break;
  }
}

UINT OpenGLAPI::GetTexelByteCount(TexelType type) {
  switch (type) {
  case TexelType::ARGB8:
  case TexelType::DEPTH32F:
    return 4;
  case TexelType::ARGB16:
  case TexelType::ARGB16F:
    return 8;
  case TexelType::ARGB32F:
    return 16;
  }
  SHOULD_NOT_HAPPEN;
  return 0;
}


std::shared_ptr<Texture> OpenGLAPI::MakeTexture(int width, int height, TexelType type,
  const void* texelData, bool gpuMemoryOnly, bool isMultisample,
  bool doesRepeat, bool generateMipmaps)
{
  ASSERT(!PleaseNoNewResources);
  ASSERT(!(texelData != nullptr && isMultisample));
  ASSERT(!(texelData == nullptr && generateMipmaps));

  CheckGLError();
  GLuint handle;
  glGenTextures(1, &handle);
  SetActiveTexture(0);
  CheckGLError();

  if (isMultisample) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, handle);
    CheckGLError();
    GLint internalFormat;
    GLenum format, glType;
    GetTextureType(type, internalFormat, format, glType);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
      ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT,
      internalFormat, width, height, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  }
  else {
    BindTexture(handle);
    if (type == TexelType::DEPTH32F) {
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else {
      if (generateMipmaps) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
      }
      else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    const auto wrapMode = doesRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    SetTextureData(width, height, type, texelData, generateMipmaps);
  }

  CheckGLError();
  std::shared_ptr<std::vector<char>> texelVector = nullptr;
  if (!gpuMemoryOnly) {
    texelVector = std::make_shared<std::vector<char>>();
    texelVector->resize(width * height * 4);
    memcpy(&(*texelVector)[0], texelData, width * height * 4);
  }
  return std::make_shared<Texture>(handle, width, height, type,
    texelVector, isMultisample, doesRepeat, generateMipmaps);
}


void OpenGLAPI::SetTextureData(UINT width, UINT height, TexelType type,
  const void* texelData, bool generateMipmap) {
  ASSERT(!PleaseNoNewResources);
  GLint internalFormat;
  GLenum format;
  GLenum glType;
  GetTextureType(type, internalFormat, format, glType);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, glType,
    texelData);
  if (generateMipmap) glGenerateMipmap(GL_TEXTURE_2D);
  CheckGLError();
}


void OpenGLAPI::SetTextureSubData(UINT x, UINT y, UINT width, UINT height,
  TexelType type, void* texelData) {
  ASSERT(!PleaseNoNewResources);
  GLint internalFormat;
  GLenum format;
  GLenum glType;
  GetTextureType(type, internalFormat, format, glType);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format, glType, texelData);
  glGenerateMipmap(GL_TEXTURE_2D);
  CheckGLError();
}


void OpenGLAPI::DeleteTextureGpuData(Texture::Handle handle) {
  glDeleteTextures(1, &handle);
  CheckGLError();
}


void OpenGLAPI::UploadTextureGpuData(const std::shared_ptr<Texture>& texture,
  void* texelData) {
  ASSERT(!PleaseNoNewResources);
  ASSERT(!texture->mTexelData);
  SetActiveTexture(0);
  BindTexture(texture->mHandle);
  SetTextureData(texture->mWidth, texture->mHeight, texture->mType, texelData,
    texture->mGenerateMipmaps);
  CheckGLError();
}


void OpenGLAPI::SetTexture(const ShaderProgram::Sampler& sampler,
  const std::shared_ptr<Texture>& texture, UINT slotIndex)
{
  CheckGLError();
  SetActiveTexture(slotIndex);
  if (!texture) {
    BindTexture(0);
  }
  else {
    if (texture->mIsMultisample) BindMultisampleTexture(texture->mHandle);
    else BindTexture(texture->mHandle);
  }
  glUniform1i(sampler.mHandle, slotIndex);
  CheckGLError();
}


FrameBufferId OpenGLAPI::CreateFrameBuffer(const std::shared_ptr<Texture>& depthBuffer,
  const std::shared_ptr<Texture>& targetBufferA,
  const std::shared_ptr<Texture>& targetBufferB)
{
  ASSERT(!PleaseNoNewResources);
  ASSERT(!targetBufferA || !depthBuffer ||
    depthBuffer->mIsMultisample == targetBufferA->mIsMultisample);
  CheckGLError();
  GLuint bufferId;
  glCreateFramebuffers(1, &bufferId);
  //BindFrameBuffer(bufferId);
  CheckGLError();

  if (depthBuffer) {
    glNamedFramebufferTexture(bufferId, GL_DEPTH_ATTACHMENT, depthBuffer->mHandle, 0);
  }

  if (!targetBufferA) {
    /// No target buffer
  }
  else if (!targetBufferB) {
    CheckGLError();
    glNamedFramebufferTexture(bufferId, GL_COLOR_ATTACHMENT0, targetBufferA->mHandle, 0);
    GLuint attachments[] = { GL_COLOR_ATTACHMENT0 };
    CheckGLError();
    glNamedFramebufferDrawBuffers(bufferId, 1, attachments);
    CheckGLError();
  }
  else {
    CheckGLError();
    glNamedFramebufferTexture(bufferId, GL_COLOR_ATTACHMENT0, targetBufferA->mHandle, 0);
    glNamedFramebufferTexture(bufferId, GL_COLOR_ATTACHMENT1, targetBufferB->mHandle, 0);
    GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    CheckGLError();
    glNamedFramebufferDrawBuffers(bufferId, 2, attachments);
    CheckGLError();
  }
  CheckGLError();

  if (targetBufferA) {
    glNamedFramebufferReadBuffer(bufferId, GL_COLOR_ATTACHMENT0);
  }

  const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    ERR("Framebuffer incomplete, status: 0x%x\n", status);
  }

  CheckGLError();
  return bufferId;
}


void OpenGLAPI::DeleteFrameBuffer(FrameBufferId frameBufferId) {
  glDeleteFramebuffers(1, &frameBufferId);
  CheckGLError();
}


void OpenGLAPI::SetFrameBuffer(FrameBufferId frameBufferId) {
  BindFrameBuffer(frameBufferId);
  CheckGLError();
}


void OpenGLAPI::BlitFrameBuffer(FrameBufferId source, FrameBufferId target,
  int srcX0, int srcY0, int srcX1, int srcY1,
  int dstX0, int dstY0, int dstX1, int dstY1) 
{
  CheckGLError();
  glBlitNamedFramebuffer(source, target, 
                         srcX0, srcY0, srcX1, srcY1,
                         dstX0, dstY0, dstX1, dstY1,
                         GL_COLOR_BUFFER_BIT, GL_LINEAR);
  CheckGLError();
}


void OpenGLAPI::SetActiveTexture(UINT activeTextureIndex) {
  if (mActiveTextureShadow == activeTextureIndex) return;
  glActiveTexture(GL_TEXTURE0 + activeTextureIndex);
  mActiveTextureShadow = activeTextureIndex;
  CheckGLError();
}


void OpenGLAPI::BindTexture(Texture::Handle textureId) {
  if (mBoundTextureShadow[mActiveTextureShadow] == textureId) return;
  glBindTexture(GL_TEXTURE_2D, textureId);
  mBoundTextureShadow[mActiveTextureShadow] = textureId;
  CheckGLError();
}


void OpenGLAPI::BindMultisampleTexture(Texture::Handle textureId) {
  if (mBoundMultisampleTextureShadow[mActiveTextureShadow] == textureId) return;
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureId);
  mBoundMultisampleTextureShadow[mActiveTextureShadow] = textureId;
  CheckGLError();
}


void OpenGLAPI::SetVertexBuffer(const std::shared_ptr<Buffer>& buffer) {
  BindVertexBuffer(buffer->GetHandle());
}


void OpenGLAPI::SetIndexBuffer(const std::shared_ptr<Buffer>& buffer) {
  BindIndexBuffer(buffer->GetHandle());
}


void OpenGLAPI::SetSsbo(UINT index, const std::shared_ptr<Buffer>& buffer) {
  if (!buffer) return;
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer->GetHandle());
}

void OpenGLAPI::EnableVertexAttribute(UINT index, ValueType nodeType, UINT offset,
  UINT stride) 
{
  GLint size = 0;
  GLenum type = 0;
  switch (nodeType) {
  case ValueType::FLOAT:  size = 1;	type = GL_FLOAT;	break;
  case ValueType::VEC2:   size = 2;	type = GL_FLOAT;	break;
  case ValueType::VEC3:   size = 3;	type = GL_FLOAT;	break;
  case ValueType::VEC4:   size = 4;	type = GL_FLOAT;	break;
  default:
    ERR(L"Unhandled vertex attribute type");
    break;
  }
  CheckGLError();
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, type, GL_FALSE, stride, 
    reinterpret_cast<void*>(size_t(offset)));
  CheckGLError();
}

ShaderProgram::ShaderProgram(ShaderHandle shaderHandle, 
  ShaderHandle vertexProgramHandle, ShaderHandle fragmentProgramHandle,
  std::vector<Uniform>& uniforms, std::vector<Sampler>& samplers, std::vector<SSBO>& ssbos,
  UINT uniformBlockSize)
  : mProgramHandle(shaderHandle)
  , mVertexShaderHandle(vertexProgramHandle)
  , mFragmentShaderHandle(fragmentProgramHandle)
  , mUniforms(uniforms)
  , mSamplers(samplers)
  , mSSBOs(ssbos)
  , mUniformBlockSize(uniformBlockSize) {}

ShaderProgram::~ShaderProgram() {
  CheckGLError();
  glDeleteProgram(mProgramHandle);
  CheckGLError();
  glDeleteShader(mVertexShaderHandle);
  CheckGLError();
  glDeleteShader(mFragmentShaderHandle);
  CheckGLError();
}

ShaderProgram::Uniform::Uniform(std::string name, ValueType type, UINT offset)
  : mName(std::move(name))
  , mType(type)
  , mOffset(offset) {}

ShaderProgram::Sampler::Sampler(std::string name, SamplerId handle)
  : mName(std::move(name))
  , mHandle(handle) {}

Buffer::Buffer(int byteSize) {
  if (byteSize >= 0) {
    Allocate(byteSize);
  }
}

Buffer::~Buffer() {
  Release();
}

void Buffer::Allocate(int byteSize) {
  CheckGLError();
  if (mHandle == 0) {
    glCreateBuffers(1, &mHandle);
    CheckGLError();
  }
  if (byteSize != mByteSize) {
    glNamedBufferData(mHandle, byteSize, nullptr, GL_DYNAMIC_DRAW);
    CheckGLError();
    mByteSize = byteSize;
  }
}

bool Buffer::IsEmpty() const
{
  return mHandle == 0 || mByteSize <= 0;
}

void Buffer::UploadData(const void* data, int byteSize) {
  CheckGLError();
  if (!mHandle) {
    ASSERT(mByteSize == -1);
    Allocate(-1);
  }
  if (mByteSize < byteSize) {
    glNamedBufferData(mHandle, byteSize, data, GL_DYNAMIC_DRAW);
    mByteSize = byteSize;
  }
  else {
    /// Don't realloc buffers when the size is appropriate
    glNamedBufferSubData(mHandle, 0, byteSize, data);
  }
  CheckGLError();
}

DrawingAPIHandle Buffer::GetHandle() const
{
  return mHandle;
}

int Buffer::GetByteSize() const
{
  return mByteSize;
}

void Buffer::Release() {
  CheckGLError();
  if (mHandle == 0) return;
  glDeleteBuffers(1, &mHandle);
  mHandle = 0;
  mByteSize = -1;
  CheckGLError();
}

ShaderProgram::SSBO::SSBO(std::string name, UINT index)
  : mName(std::move(name))
  , mIndex(index)
{}
