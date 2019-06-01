﻿#include <include/render/drawingapi.h>
#include <include/base/helpers.h>
#include <Windows.h>

#define GLEW_STATIC
#include <glew/glew.h>

#ifdef _DEBUG
void CheckGLError() {
  GLenum error = glGetError();
  ASSERT(error == GL_NO_ERROR);
}
#else
#	define CheckGLError()
#endif

struct MappedAttributeOpenGL {
  GLuint Index;
  GLint Size;
  GLenum Type;
  UINT Offset;
};


class AttributeMapperOpenGL : public AttributeMapper {
public:
  AttributeMapperOpenGL();
  virtual ~AttributeMapperOpenGL() {}

  void Set() const;

  GLsizei Stride;

  vector<MappedAttributeOpenGL>	MappedAttributes;
};


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
  {NULL, NULL}
};

OpenGLAPI::OpenGLAPI() {
  GLenum err = glewInit();
  ASSERT(err == GLEW_OK);
  if (err != GLEW_OK) {
    ERR(L"Cannot initialize OpenGL.");
    SHOULD_NOT_HAPPEN;
  }
  else {
    const wchar_t* versionName = NULL;
    for (GLVersion* version = gOpenGLVersions; version->version != NULL; version++) {
      if (*version->version) versionName = version->name;
    }

    if (versionName == NULL) ERR(L"OpenGL not found at all.");
    else INFO(L"OpenGL version %s found.", versionName);

    if (!GLEW_VERSION_4_3) {
      ERR(L"Sorry, OpenGL 4.3 needed at least.");
    }
    CheckGLError();
  }

  //glEnable(GL_MULTISAMPLE);
  OnContextSwitch();
  CheckGLError();
}


OpenGLAPI::~OpenGLAPI() {}


void OpenGLAPI::OnContextSwitch() {
  /// Set defaults (shadow values must be something different at the beginning 
  /// to avoid false cache hit)
  mFaceMode = RenderState::FaceMode::BACK;
  SetFaceMode(RenderState::FaceMode::FRONT_AND_BACK);

  mBlendMode = RenderState::BlendMode::ADDITIVE;
  mBlendEnabled = true;
  SetBlendMode(RenderState::BlendMode::NORMAL);

  DepthTestEnabledShadow = true;
  SetDepthTest(false);

  ClearColorShadow = 1;
  SetClearColor(0);

  glDepthMask(true);

  ActiveTextureShadow = -1;
  BoundVertexBufferShadow = -1;
  BoundIndexBufferShadow = -1;
  BoundFrameBufferShadow = -1;
  BoundFrameBufferReadBuffer = -1;
  BoundFrameBufferDrawBuffer = -1;
  for (int i = 0; i < MAX_COMBINED_TEXTURE_SLOTS; i++) {
    BoundTextureShadow[i] = (GLuint)-1;
    BoundMultisampleTextureShadow[i] = (GLuint)-1;
  }
  CheckGLError();
}

static ShaderHandle CompileAndAttachShader(GLuint program, GLuint shaderType,
  const char* source) {
  ASSERT(!PleaseNoNewResources);

  CheckGLError();
  /// Create shader object, set the source, and compile
  GLuint shader = glCreateShader(shaderType);
  GLint length = GLint(strlen(source));
  glShaderSource(shader, 1, (const char **)&source, &length);
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

/// Array for attribute names
const char* gVertexAttributeName[] = {
  "aPosition",
  "aTexCoord",
  "aNormal",
  "aTangent",
};

shared_ptr<ShaderProgram> OpenGLAPI::CreateShaderFromSource(
  const char* vertexSource, const char* fragmentSource) {
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
    CheckGLError();
    glDeleteShader(vertexShaderHandle);
    CheckGLError();
    return nullptr;
  }

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
    delete log;
  }

  if (result == GL_FALSE) {
    glDeleteProgram(program);
    glDeleteShader(vertexShaderHandle);
    glDeleteShader(fragmentShaderHandle);
    CheckGLError();
    return nullptr;
  }

  /// Query uniform blocks
  GLint uniformBlockCount;
  glGetProgramInterfaceiv(program, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES,
    &uniformBlockCount);
  /// There should only be "Uniforms"
  ASSERT(uniformBlockCount == 1);

  /// And it should be at index 0
  int uniformBlockIndex = glGetUniformBlockIndex(program, "Uniforms");
  ASSERT(uniformBlockIndex == 0);

  /// This is how OpenGL interface API works. Beautiful like a megmikrózott kefir.
  /// Get the number of active uniforms inside the uniform block, and the block size.
  const GLenum blockPropsList[] = { GL_NUM_ACTIVE_VARIABLES, GL_BUFFER_DATA_SIZE };
#pragma pack(push, 1)
  struct { GLint mUniformCount, mSize; } blockProps;
#pragma pack(pop)
  glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, uniformBlockIndex,
    ElementCount(blockPropsList), blockPropsList,
    ElementCount(blockPropsList), nullptr,
    reinterpret_cast<GLint*>(&blockProps));

  //if (numActiveUnifs > 0) continue;
  vector<ShaderProgram::Uniform> uniforms;
  uniforms.reserve(blockProps.mUniformCount);

  std::vector<GLint> uniformLocations(blockProps.mUniformCount);
  const GLenum activeUnifProp[1] = { GL_ACTIVE_VARIABLES };
  glGetProgramResourceiv(program, GL_UNIFORM_BLOCK, uniformBlockIndex, 1, activeUnifProp,
    blockProps.mUniformCount, nullptr, &uniformLocations[0]);

  /// Query the properties of each uniform inside the block
  const GLenum uniformProperties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_OFFSET };
#pragma pack(push, 1)
  struct { GLint mNameLength, mType, mLocation, mOffset; } values;
#pragma pack(pop)

  for (int blockIndex = 0; blockIndex < blockProps.mUniformCount; ++blockIndex) {
    glGetProgramResourceiv(program, GL_UNIFORM, uniformLocations[blockIndex],
      ElementCount(uniformProperties), uniformProperties,
      ElementCount(uniformProperties), nullptr,
      reinterpret_cast<GLint*>(&values));

    /// Get the name
    vector<char> name(values.mNameLength);
    glGetProgramResourceName(program, GL_UNIFORM, uniformLocations[blockIndex],
      values.mNameLength, nullptr, &name[0]);

    ShaderValueType nodeType;
    switch (values.mType) {
    case GL_FLOAT:		  nodeType = ShaderValueType::FLOAT;		break;
    case GL_FLOAT_VEC2:	nodeType = ShaderValueType::VEC2;		  break;
    case GL_FLOAT_VEC3:	nodeType = ShaderValueType::VEC3;		  break;
    case GL_FLOAT_VEC4:	nodeType = ShaderValueType::VEC4;		  break;
    case GL_FLOAT_MAT4:	nodeType = ShaderValueType::MATRIX44;	break;
    default: SHOULD_NOT_HAPPEN; break;
    }

    uniforms.push_back(
      ShaderProgram::Uniform(string(&name[0]), nodeType, values.mOffset));
  }

  /// Query samplers' list with the old OpenGL API
  /// Interface API can't handle samplers :((((
  vector<ShaderProgram::Sampler> samplers;
  GLint uniformCount;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

  GLint uniformNameMaxLength;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameMaxLength);
  vector<char> samplerName(uniformNameMaxLength);

  for (int uniformIndex = 0; uniformIndex < uniformCount; uniformIndex++) {
    /// Request info about Nth uniform
    GLint nameLength;
    GLsizei size;
    GLenum type;
    glGetActiveUniform(program, uniformIndex, uniformNameMaxLength, &nameLength, &size,
      &type, &samplerName[0]);
    if (type == GL_SAMPLER_2D || type == GL_SAMPLER_2D_MULTISAMPLE ||
      type == GL_SAMPLER_2D_SHADOW) {
      GLint location = glGetUniformLocation(program, &samplerName[0]);
      samplers.push_back(ShaderProgram::Sampler(string(&samplerName[0]), location));
    }
  }
  CheckGLError();

  /// Create attributes list
  GLint attributeCount;
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);

  vector<ShaderProgram::Attribute> attributes;
  attributes.reserve(attributeCount);

  GLint attributeNameMaxLength;
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);
  vector<char> attributeName(attributeNameMaxLength);

  for (int uniformIndex = 0; uniformIndex < attributeCount; uniformIndex++) {
    /// Request info about Nth attribute
    GLint nameLength;
    GLsizei size;
    GLenum type;

    glGetActiveAttrib(program, uniformIndex, attributeNameMaxLength, &nameLength, &size,
      &type, &attributeName[0]);

    /// COME ON OPENGL, FUCK YOU, WHY CANT THE LOCATION JUST BE THE INDEX.
    AttributeId location = glGetAttribLocation(program, &attributeName[0]);

    /// Shader compiler reports gl_InstanceID as an attribute at -1, who knows why.
    if (location < 0) continue;

    ShaderValueType valueType;
    switch (type) {
    case GL_FLOAT:		  valueType = ShaderValueType::FLOAT;		break;
    case GL_FLOAT_VEC2:	valueType = ShaderValueType::VEC2;		break;
    case GL_FLOAT_VEC3:	valueType = ShaderValueType::VEC3;		break;
    case GL_FLOAT_VEC4:	valueType = ShaderValueType::VEC4;		break;
    default: SHOULD_NOT_HAPPEN; break;
    }

    /// Map attribute name to usage
    bool found = false;
    VertexAttributeUsage usage = VertexAttributeUsage::NONE;
    for (UINT o = 0; o < (UINT)VertexAttributeUsage::COUNT; o++) {
      if (strcmp(gVertexAttributeName[o], &attributeName[0]) == 0) {
        usage = VertexAttributeUsage(o);
        found = true;
        break;
      }
    }
    if (!found) {
      ERR("Unrecognized vertex attribute name: %s", attributeName);
    }

    attributes.push_back(
      ShaderProgram::Attribute(string(&attributeName[0]), valueType, location, usage));
  }
  CheckGLError();

  /// Create a uniform buffer object for the shader
  GLuint uboHandle;
  glGenBuffers(1, &uboHandle);
  glBindBuffer(GL_UNIFORM_BUFFER, uboHandle);
  glBufferData(GL_UNIFORM_BUFFER, blockProps.mSize, nullptr, GL_DYNAMIC_DRAW);
  CheckGLError();

  return make_shared<ShaderProgram>(program, vertexShaderHandle, fragmentShaderHandle,
    uniforms, samplers, attributes, blockProps.mSize,
    uboHandle);
}


void OpenGLAPI::SetShaderProgram(const shared_ptr<ShaderProgram>& program,
  void* uniforms) {
  CheckGLError();
  glUseProgram(program->mProgramHandle);
  CheckGLError();

  glBindBuffer(GL_UNIFORM_BUFFER, program->mUniformBufferHandle);
  glBufferData(GL_UNIFORM_BUFFER, program->mUniformBlockSize, uniforms, GL_DYNAMIC_DRAW);
  CheckGLError();

  glBindBufferBase(GL_UNIFORM_BUFFER, 0, program->mUniformBufferHandle);
  CheckGLError();
}


void OpenGLAPI::SetUniform(UniformId id, ShaderValueType type, const void* values) {
  CheckGLError();

  switch (type) {
  case ShaderValueType::FLOAT:
    glUniform1f(id, *(const GLfloat*)values);
    break;
  case ShaderValueType::VEC2:
    glUniform2fv(id, 1, (const GLfloat*)values);
    break;
  case ShaderValueType::VEC3:
    glUniform3fv(id, 1, (const GLfloat*)values);
    break;
  case ShaderValueType::VEC4:
    glUniform4fv(id, 1, (const GLfloat*)values);
    break;
  case ShaderValueType::MATRIX44:
    glUniformMatrix4fv(id, 1, false, (const GLfloat*)values);
    break;
  default:
    SHOULD_NOT_HAPPEN;
    break;
  }
  CheckGLError();
}


VertexBufferHandle OpenGLAPI::CreateVertexBuffer(UINT size) {
  ASSERT(!PleaseNoNewResources);
  VertexBufferHandle handle;
  glGenBuffers(1, &handle);
  CheckGLError();
  BindVertexBuffer(handle);
  glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
  CheckGLError();
  return handle;
}


void OpenGLAPI::DestroyVertexBuffer(VertexBufferHandle handle) {
  CheckGLError();
  glDeleteBuffers(1, &handle);
  CheckGLError();
}


void* OpenGLAPI::MapVertexBuffer(VertexBufferHandle handle) {
  ASSERT(!PleaseNoNewResources);
  BindVertexBuffer(handle);
  void* address = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  CheckGLError();
  return address;
}


void OpenGLAPI::UnMapVertexBuffer(VertexBufferHandle handle) {
  /// Please don't do anything else while a buffer is mapped
  ASSERT(BoundVertexBufferShadow == handle);
  glUnmapBuffer(GL_ARRAY_BUFFER);
  CheckGLError();
}


IndexBufferHandle OpenGLAPI::CreateIndexBuffer(UINT size) {
  ASSERT(!PleaseNoNewResources);
  IndexBufferHandle handle;
  glGenBuffers(1, &handle);
  CheckGLError();
  BindIndexBuffer(handle);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
  CheckGLError();
  return handle;
}


void OpenGLAPI::DestroyIndexBuffer(IndexBufferHandle handle) {
  glDeleteBuffers(1, &handle);
  CheckGLError();
}


void* OpenGLAPI::MapIndexBuffer(IndexBufferHandle handle) {
  ASSERT(!PleaseNoNewResources);
  BindIndexBuffer(handle);
  void* buffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  CheckGLError();
  return buffer;
}


void OpenGLAPI::UnMapIndexBuffer(IndexBufferHandle handle) {
  /// Please don't do anything else while a buffer is mapped
  ASSERT(BoundIndexBufferShadow == handle);
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  CheckGLError();
}


void OpenGLAPI::BindVertexBuffer(GLuint bufferID) {
  if (bufferID != BoundVertexBufferShadow) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    CheckGLError();
    BoundVertexBufferShadow = bufferID;
  }
}


void OpenGLAPI::BindIndexBuffer(GLuint bufferID) {
  if (bufferID != BoundIndexBufferShadow) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    CheckGLError();
    BoundIndexBufferShadow = bufferID;
  }
}


void OpenGLAPI::BindFrameBuffer(GLuint frameBufferID) {
  if (frameBufferID != BoundFrameBufferShadow) {
    CheckGLError();
    glBindFramebuffer(GL_FRAMEBUFFER, frameBufferID);
    CheckGLError();
    BoundFrameBufferShadow = frameBufferID;
  }
}


AttributeMapper* OpenGLAPI::CreateAttributeMapper(
  const vector<VertexAttribute>& bufferAttribs,
  const vector<ShaderProgram::Attribute>& shaderAttribs, UINT stride) {
  ASSERT(!PleaseNoNewResources);
  AttributeMapperOpenGL* mapper = new AttributeMapperOpenGL();
  mapper->Stride = stride;

  for (const ShaderProgram::Attribute& shaderAttr : shaderAttribs) {
    bool found = false;
    for (const VertexAttribute& bufferAttr : bufferAttribs) {
      if (bufferAttr.Usage == shaderAttr.mUsage) {
        MappedAttributeOpenGL attr;
        attr.Index = shaderAttr.mHandle;
        switch (VertexAttributeUsageToValueType(bufferAttr.Usage)) {
        case ShaderValueType::FLOAT:		attr.Size = 1;	attr.Type = GL_FLOAT;	break;
        case ShaderValueType::VEC2:		attr.Size = 2;	attr.Type = GL_FLOAT;	break;
        case ShaderValueType::VEC3:		attr.Size = 3;	attr.Type = GL_FLOAT;	break;
        case ShaderValueType::VEC4:		attr.Size = 4;	attr.Type = GL_FLOAT;	break;
        default:
          ERR(L"Unhandled vertex attribute type");
          break;
        }
        attr.Offset = bufferAttr.Offset;
        mapper->MappedAttributes.push_back(attr);
        found = true;
        break;
      }
    }
    if (!found) {
      ERR(L"Shader needs a vertex attribute that's missing from the buffer.");
    }
  }
  return mapper;
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


void OpenGLAPI::RenderIndexedMesh(IndexBufferHandle indexHandle,
  UINT indexCount, VertexBufferHandle vertexHandle,
  const AttributeMapper* mapper,
  PrimitiveTypeEnum primitiveType) {
  BindVertexBuffer(vertexHandle);
  static_cast<const AttributeMapperOpenGL*>(mapper)->Set();

  BindIndexBuffer(indexHandle);
  glDrawElements(GetGLPrimitive(primitiveType), indexCount, GL_UNSIGNED_INT, NULL);
  CheckGLError();
}


void OpenGLAPI::RenderMesh(VertexBufferHandle vertexHandle, UINT vertexCount,
  const AttributeMapper* mapper,
  PrimitiveTypeEnum primitiveType) {
  BindVertexBuffer(vertexHandle);
  static_cast<const AttributeMapperOpenGL*>(mapper)->Set();
  glDrawArrays(GetGLPrimitive(primitiveType), 0, vertexCount);
  CheckGLError();
}


void OpenGLAPI::Render(IndexBufferHandle indexBuffer, UINT count,
  PrimitiveTypeEnum primitiveType, UINT InstanceCount) {
  CheckGLError();
  if (indexBuffer != 0) {
    BindIndexBuffer(indexBuffer);
    CheckGLError();
    glDrawElementsInstanced(
      GetGLPrimitive(primitiveType), count, GL_UNSIGNED_INT, NULL, InstanceCount);
  }
  else {
    glDrawArraysInstanced(GetGLPrimitive(primitiveType), 0, count, InstanceCount);
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
  if (enable == DepthTestEnabledShadow) return;
  if (enable) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
  }
  else glDisable(GL_DEPTH_TEST);
  DepthTestEnabledShadow = enable;
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


void OpenGLAPI::BindReadFramebuffer(GLuint framebuffer) {
  if (BoundFrameBufferReadBuffer == framebuffer) return;
  BoundFrameBufferReadBuffer = framebuffer;
  CheckGLError();
  ASSERT(glIsFramebuffer(framebuffer));
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
  CheckGLError();
}


void OpenGLAPI::BindDrawFramebuffer(GLuint framebuffer) {
  if (BoundFrameBufferDrawBuffer == framebuffer) return;
  BoundFrameBufferDrawBuffer = framebuffer;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
  CheckGLError();
}


void OpenGLAPI::NamedFramebufferReadBuffer(GLuint framebuffer, GLenum mode) {
  BindReadFramebuffer(framebuffer);
  glReadBuffer(mode);
  CheckGLError();
}


void OpenGLAPI::NamedFramebufferDrawBuffer(GLuint framebuffer, GLenum mode) {
  BindDrawFramebuffer(framebuffer);
  glDrawBuffer(mode);
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
  if (clearColor == ClearColorShadow) return;
  glClearColor(
    IntColorToFloat((clearColor >> 16) & 0xff),
    IntColorToFloat((clearColor >> 8) & 0xff),
    IntColorToFloat((clearColor) & 0xff),
    1.0f);
  ClearColorShadow = clearColor;
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


shared_ptr<Texture> OpenGLAPI::MakeTexture(int width, int height, TexelType type,
  const shared_ptr<vector<char>>& texelData, bool gpuMemoryOnly, bool isMultisample,
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
    auto wrapMode = doesRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    void* data = texelData ? &(*texelData)[0] : nullptr;
    SetTextureData(width, height, type, data, generateMipmaps);
  }

  CheckGLError();
  return make_shared<Texture>(handle, width, height, type,
    gpuMemoryOnly ? nullptr : texelData, isMultisample, doesRepeat, generateMipmaps);
}



//TextureHandle OpenGLAPI::CreateTexture(int width, int height, TexelType type,
//                                       bool isMultiSample, bool doesRepeat, bool mipmap) {
//  ASSERT(!PleaseNoNewResources);
//  CheckGLError();
//  GLuint texture;
//  glGenTextures(1, &texture);
//  SetActiveTexture(0);
//  CheckGLError();
//
//  if (isMultiSample) {
//    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
//    CheckGLError();
//    GLint internalFormat;
//    GLenum format, glType;
//    GetTextureType(type, internalFormat, format, glType);
//    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
//                            ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT,
//                            internalFormat, width, height, false);
//    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
//  } else {
//    BindTexture(texture);
//    if (type == TexelType::DEPTH32F) {
//      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//    } else {
//      if (mipmap) {
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
//      } else {
//        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//      }
//      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    }
//    auto wrapMode = doesRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
//    SetTextureData(width, height, type, NULL, mipmap);
//  }
//  CheckGLError();
//  return texture;
//}


void OpenGLAPI::SetTextureData(UINT width, UINT height, TexelType type,
  void* texelData, bool generateMipmap) {
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


void OpenGLAPI::DeleteTextureGPUData(Texture::Handle handle) {
  glDeleteTextures(1, &handle);
  CheckGLError();
}


void OpenGLAPI::UploadTextureGPUData(const shared_ptr<Texture>& texture,
  void* texelData) {
  ASSERT(!PleaseNoNewResources);
  ASSERT(!texture->mTexelData);
  SetActiveTexture(0);
  BindTexture(texture->mHandle);
  SetTextureData(texture->mWidth, texture->mHeight, texture->mType, texelData,
    texture->mGenerateMipmaps);
  CheckGLError();
}


//void OpenGLAPI::UploadTextureSubData(TextureHandle handle, UINT x, UINT y,
//                                     int width, int height, TexelType type,
//                                     void* texelData) {
//  ASSERT(!PleaseNoNewResources);
//  BindTexture(handle);
//  SetTextureSubData(width, x, y, height, type, texelData);
//  CheckGLError();
//}


void OpenGLAPI::SetTexture(const ShaderProgram::Sampler& sampler,
  const shared_ptr<Texture>& texture, UINT slotIndex)
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


FrameBufferId OpenGLAPI::CreateFrameBuffer(const shared_ptr<Texture>& depthBuffer,
  const shared_ptr<Texture>& targetBufferA,
  const shared_ptr<Texture>& targetBufferB)
{
  ASSERT(!PleaseNoNewResources);
  ASSERT(!targetBufferA || !depthBuffer ||
    depthBuffer->mIsMultisample == targetBufferA->mIsMultisample);
  CheckGLError();
  GLuint bufferId;
  glGenFramebuffers(1, &bufferId);
  BindFrameBuffer(bufferId);
  CheckGLError();

  bool isMultisample = (depthBuffer ? depthBuffer : targetBufferA)->mIsMultisample;
  GLenum target = isMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

  if (depthBuffer) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target,
      depthBuffer->mHandle, 0);
  }

  if (!targetBufferA) {
    /// No target buffer
  }
  else if (!targetBufferB) {
    CheckGLError();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target,
      targetBufferA->mHandle, 0);
    GLuint attachments[] = { GL_COLOR_ATTACHMENT0 };
    CheckGLError();
    glDrawBuffers(1, attachments);
    CheckGLError();
  }
  else {
    CheckGLError();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target,
      targetBufferA->mHandle, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, target,
      targetBufferB->mHandle, 0);
    GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    CheckGLError();
    glDrawBuffers(2, attachments);
    CheckGLError();
  }
  CheckGLError();

  if (targetBufferA) {
    NamedFramebufferReadBuffer(bufferId, GL_COLOR_ATTACHMENT0);
  }

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
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


void OpenGLAPI::SetFrameBuffer(FrameBufferId frameBufferid) {
  BindFrameBuffer(frameBufferid);
  CheckGLError();
}


void OpenGLAPI::BlitFrameBuffer(FrameBufferId source, FrameBufferId target,
  int srcX0, int srcY0, int srcX1, int srcY1,
  int dstX0, int dstY0, int dstX1, int dstY1) {

  CheckGLError();
  BindReadFramebuffer(source);
  BindDrawFramebuffer(target);
  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
    dstX0, dstY0, dstX1, dstY1,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  //glBlitNamedFramebuffer(source, target, 
  //                       srcX0, srcY0, srcX1, srcY1,
  //                       dstX0, dstY0, dstX1, dstY1,
  //                       GL_COLOR_BUFFER_BIT, GL_LINEAR);
  CheckGLError();
}


void OpenGLAPI::SetActiveTexture(GLuint activeTextureIndex) {
  if (ActiveTextureShadow == activeTextureIndex) return;
  glActiveTexture(GL_TEXTURE0 + activeTextureIndex);
  ActiveTextureShadow = activeTextureIndex;
  CheckGLError();
}


void OpenGLAPI::BindTexture(GLuint textureID) {
  if (BoundTextureShadow[ActiveTextureShadow] == textureID) return;
  glBindTexture(GL_TEXTURE_2D, textureID);
  BoundTextureShadow[ActiveTextureShadow] = textureID;
  CheckGLError();
}


void OpenGLAPI::BindMultisampleTexture(GLuint textureID) {
  if (BoundMultisampleTextureShadow[ActiveTextureShadow] == textureID) return;
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
  BoundMultisampleTextureShadow[ActiveTextureShadow] = textureID;
  CheckGLError();
}


void OpenGLAPI::SetVertexBuffer(VertexBufferHandle handle) {
  BindVertexBuffer(handle);
}


void OpenGLAPI::SetIndexBuffer(IndexBufferHandle handle) {
  BindIndexBuffer(handle);
}


void OpenGLAPI::EnableVertexAttribute(UINT index, ShaderValueType nodeType, UINT offset,
  UINT stride) {
  GLint size = 0;
  GLenum type = 0;
  switch (nodeType) {
  case ShaderValueType::FLOAT:  size = 1;	type = GL_FLOAT;	break;
  case ShaderValueType::VEC2:   size = 2;	type = GL_FLOAT;	break;
  case ShaderValueType::VEC3:   size = 3;	type = GL_FLOAT;	break;
  case ShaderValueType::VEC4:   size = 4;	type = GL_FLOAT;	break;
  default:
    ERR(L"Unhandled vertex attribute type");
    break;
  }
  CheckGLError();
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)size_t(offset));
  CheckGLError();
}


AttributeMapperOpenGL::AttributeMapperOpenGL() {
  Stride = 0;
}


void AttributeMapperOpenGL::Set() const {
  for (const MappedAttributeOpenGL& attr : MappedAttributes) {
    glEnableVertexAttribArray(attr.Index);
    glVertexAttribPointer(attr.Index, attr.Size, attr.Type, GL_FALSE, Stride, (void*)size_t(attr.Offset));
    CheckGLError();
  }
}

ShaderProgram::ShaderProgram(ShaderHandle shaderHandle, ShaderHandle vertexProgramHandle,
  ShaderHandle fragmentProgramHandle,
  vector<Uniform>& uniforms, vector<Sampler>& samplers,
  vector<Attribute>& attributes, UINT uniformBlockSize,
  ShaderHandle uniformBufferHandle)
  : mProgramHandle(shaderHandle)
  , mVertexShaderHandle(vertexProgramHandle)
  , mFragmentShaderHandle(fragmentProgramHandle)
  , mUniforms(uniforms)
  , mSamplers(samplers)
  , mAttributes(attributes)
  , mUniformBlockSize(uniformBlockSize)
  , mUniformBufferHandle(uniformBufferHandle) {}

ShaderProgram::~ShaderProgram() {
  CheckGLError();
  glDeleteProgram(mProgramHandle);
  CheckGLError();
  glDeleteShader(mVertexShaderHandle);
  CheckGLError();
  glDeleteShader(mFragmentShaderHandle);
  CheckGLError();
  glDeleteBuffers(1, &mUniformBufferHandle);
  CheckGLError();
}

ShaderProgram::Uniform::Uniform(const string& name, ShaderValueType type, UINT offset)
  : mName(name)
  , mType(type)
  , mOffset(offset) {}

ShaderProgram::Sampler::Sampler(const string& name, SamplerId handle)
  : mName(name)
  , mHandle(handle) {}

ShaderProgram::Attribute::Attribute(const string& name, ShaderValueType type,
  AttributeId handle, VertexAttributeUsage usage)
  : mName(name)
  , mType(type)
  , mHandle(handle)
  , mUsage(usage) {}
