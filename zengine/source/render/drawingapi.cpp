#include <include/render/drawingapi.h>
#include <include/base/helpers.h>
#include <Windows.h>

#define GLEW_STATIC
#include <glew/glew.h>

#ifdef _DEBUG
void CheckGLError() {
  GLenum error = glGetError();  //ASSERT(error == GL_NO_ERROR);
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


class AttributeMapperOpenGL: public AttributeMapper {
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
  } else {
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

  glEnable(GL_MULTISAMPLE);
  OnContextSwitch();
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
}


static bool CompileAndAttachShader(GLuint program, GLuint shaderType,
                                   const char* source) {
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
    return false;
  }

  glAttachShader(program, shader);
  glDeleteShader(shader);
  CheckGLError();
  return true;
}


OWNERSHIP ShaderCompileDesc* OpenGLAPI::CreateShaderFromSource(
  const char* vertexSource, const char* fragmentSource) {
  GLuint program = glCreateProgram();

  /// Compile shaders
  if (!CompileAndAttachShader(program, GL_VERTEX_SHADER, vertexSource)) {
    WARN(L"Vertex shader compilation failed.");
    glDeleteProgram(program);
    return NULL;
  }
  if (!CompileAndAttachShader(program, GL_FRAGMENT_SHADER, fragmentSource)) {
    WARN(L"Fragment shader compilation failed.");
    glDeleteProgram(program);
    return NULL;
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
    } else {
      WARN("Shader compiler: %s", log);
    }
    delete log;
  }

  if (result == GL_FALSE) {
    glDeleteProgram(program);
    return NULL;
  }

  ShaderCompileDesc* builder = new ShaderCompileDesc;
  builder->Handle = program;

  /// Create uniforms list
  GLint uniformCount;
  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

  GLint uniformNameMaxLength;
  glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformNameMaxLength);
  char* name = new char[uniformNameMaxLength];

  for (int i = 0; i < uniformCount; i++) {
    /// Request info about Nth uniform
    GLint nameLength;
    GLsizei size;
    GLenum type;
    glGetActiveUniform(program, i, uniformNameMaxLength, &nameLength, &size, &type, name);
    GLint location = glGetUniformLocation(program, name);
    //ASSERT(i == location);
    if (type == GL_SAMPLER_2D || type == GL_SAMPLER_2D_MULTISAMPLE || type == GL_SAMPLER_2D_SHADOW) {
      ShaderSamplerDesc sampler;
      sampler.Handle = location;
      sampler.Name = name;
      builder->Samplers.push_back(sampler);
    } else {
      ShaderUniformDesc uniform;
      uniform.Handle = location;
      uniform.Name = name;
      switch (type) {
        case GL_FLOAT:		uniform.UniformType = NodeType::FLOAT;		break;
        case GL_FLOAT_VEC2:	uniform.UniformType = NodeType::VEC2;		break;
        case GL_FLOAT_VEC3:	uniform.UniformType = NodeType::VEC3;		break;
        case GL_FLOAT_VEC4:	uniform.UniformType = NodeType::VEC4;		break;
        case GL_FLOAT_MAT4:	uniform.UniformType = NodeType::MATRIX44;	break;
        default: NOT_IMPLEMENTED; uniform.UniformType = (NodeType)-1; break;
      }
      builder->Uniforms.push_back(uniform);
    }
  }
  delete name;
  CheckGLError();


  /// Create attributes list
  GLint attributeCount;
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &attributeCount);

  GLint attributeNameMaxLength;
  glGetProgramiv(program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);
  name = new char[attributeNameMaxLength];

  for (int i = 0; i < attributeCount; i++) {
    /// Request info about Nth attribute
    GLint nameLength;
    GLsizei size;
    GLenum type;

    glGetActiveAttrib(program, i, attributeNameMaxLength, &nameLength, &size, &type, name);

    /// COME ON OPENGL, FUCK YOU, WHY CANT THE LOCATION JUST BE THE INDEX.
    AttributeId location = glGetAttribLocation(program, name);

    /// Shader compiler reports gl_InstanceID as an attribute at -1, who knows why.
    if (location < 0) continue;

    ShaderAttributeDesc attribute;
    attribute.Handle = location;
    attribute.Name = name;
    switch (type) {
      case GL_FLOAT:		attribute.Type = NodeType::FLOAT;		break;
      case GL_FLOAT_VEC2:	attribute.Type = NodeType::VEC2;		break;
      case GL_FLOAT_VEC3:	attribute.Type = NodeType::VEC3;		break;
      case GL_FLOAT_VEC4:	attribute.Type = NodeType::VEC4;		break;
      default: NOT_IMPLEMENTED; attribute.Type = (NodeType)-1;	break;
    }

    /// Map attribute name to usage
    bool found = false;
    for (UINT o = 0; o < (UINT)VertexAttributeUsage::COUNT; o++) {
      if (strcmp(gVertexAttributeName[o], name) == 0) {
        attribute.Usage = (VertexAttributeUsage)o;
        found = true;
        break;
      }
    }
    if (!found) {
      ERR("Unrecognized vertex attribute name: %s", name);
    }

    builder->Attributes.push_back(attribute);
  }

  CheckGLError();
  return builder;
}


void OpenGLAPI::SetShaderProgram(ShaderHandle handle) {
  glUseProgram(handle);
  CheckGLError();
}


void OpenGLAPI::SetUniform(UniformId id, NodeType type, const void* values) {
  CheckGLError();

  switch (type) {
    case NodeType::FLOAT:		  
      glUniform1f(id, *(const GLfloat*)values);					        
      break;
    case NodeType::VEC2:		  
      glUniform2fv(id, 1, (const GLfloat*)values);				      
      break;
    case NodeType::VEC3:		  
      glUniform3fv(id, 1, (const GLfloat*)values);				      
      break;
    case NodeType::VEC4:		  
      glUniform4fv(id, 1, (const GLfloat*)values);				      
      break;
    case NodeType::MATRIX44:	
      glUniformMatrix4fv(id, 1, false, (const GLfloat*)values);	
      break;
    default: 
      NOT_IMPLEMENTED; 
      break;
  }
  CheckGLError();
}


void OpenGLAPI::DestroyShaderProgram(ShaderHandle handle) {
  glDeleteProgram(handle);
  CheckGLError();
}


VertexBufferHandle OpenGLAPI::CreateVertexBuffer(UINT size) {
  VertexBufferHandle handle;
  glGenBuffers(1, &handle);
  CheckGLError();
  BindVertexBuffer(handle);
  glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
  CheckGLError();
  return handle;
}


void OpenGLAPI::DestroyVertexBuffer(VertexBufferHandle handle) {
  glDeleteBuffers(1, &handle);
}


void* OpenGLAPI::MapVertexBuffer(VertexBufferHandle handle) {
  BindVertexBuffer(handle);
  void* address = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  CheckGLError();
  return address;
}


void OpenGLAPI::UnMapVertexBuffer(VertexBufferHandle handle) {
  /// Please don't do anything else while a buffer is mapped
  ASSERT(BoundVertexBufferShadow == handle);
  glUnmapBuffer(GL_ARRAY_BUFFER);
}


IndexBufferHandle OpenGLAPI::CreateIndexBuffer(UINT size) {
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
}


void* OpenGLAPI::MapIndexBuffer(IndexBufferHandle handle) {
  BindIndexBuffer(handle);
  void* buffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  CheckGLError();
  return buffer;
}


void OpenGLAPI::UnMapIndexBuffer(IndexBufferHandle handle) {
  /// Please don't do anything else while a buffer is mapped
  ASSERT(BoundIndexBufferShadow == handle);
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
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
  const vector<ShaderAttributeDesc>& shaderAttribs, UINT stride) {
  AttributeMapperOpenGL* mapper = new AttributeMapperOpenGL();
  mapper->Stride = stride;

  for (const ShaderAttributeDesc& shaderAttr : shaderAttribs) {
    bool found = false;
    for (const VertexAttribute& bufferAttr : bufferAttribs) {
      if (bufferAttr.Usage == shaderAttr.Usage) {
        MappedAttributeOpenGL attr;
        attr.Index = shaderAttr.Handle;
        switch (gVertexAttributeType[(UINT)bufferAttr.Usage]) {
          case NodeType::FLOAT:		attr.Size = 1;	attr.Type = GL_FLOAT;	break;
          case NodeType::VEC2:		attr.Size = 2;	attr.Type = GL_FLOAT;	break;
          case NodeType::VEC3:		attr.Size = 3;	attr.Type = GL_FLOAT;	break;
          case NodeType::VEC4:		attr.Size = 4;	attr.Type = GL_FLOAT;	break;
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
  if (indexBuffer != 0) {
    BindIndexBuffer(indexBuffer);
    CheckGLError();
    glDrawElementsInstanced(
      GetGLPrimitive(primitiveType), count, GL_UNSIGNED_INT, NULL, InstanceCount);
    CheckGLError();
  } else {
    glDrawArraysInstanced(GetGLPrimitive(primitiveType), 0, count, InstanceCount);
  }
}


void OpenGLAPI::SetViewport(int x, int y, int width, int height,
                            float depthMin /*= 0.0f*/, float depthMax /*= 1.0f*/) {
  glViewport(x, y, width, height);
  glDepthRange(depthMin, depthMax);
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
}


void OpenGLAPI::SetDepthTest(bool enable) {
  if (enable == DepthTestEnabledShadow) return;
  if (enable) glEnable(GL_DEPTH_TEST);
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
}


void OpenGLAPI::BindReadFramebuffer(GLuint framebuffer) {
  if (BoundFrameBufferReadBuffer == framebuffer) return;
  BoundFrameBufferReadBuffer = framebuffer;
  glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer);
}


void OpenGLAPI::BindDrawFramebuffer(GLuint framebuffer) {
  if (BoundFrameBufferDrawBuffer == framebuffer) return;
  BoundFrameBufferDrawBuffer = framebuffer;
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
}


void OpenGLAPI::NamedFramebufferReadBuffer(GLuint framebuffer, GLenum mode) {
  BindReadFramebuffer(framebuffer);
  glReadBuffer(mode);
}


void OpenGLAPI::NamedFramebufferDrawBuffer(GLuint framebuffer, GLenum mode) {
  BindDrawFramebuffer(framebuffer);
  glDrawBuffer(mode);
}


void OpenGLAPI::SetBlending(bool enable) {
  if (enable == mBlendEnabled) return;
  if (enable) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  mBlendEnabled = enable;
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


TextureHandle OpenGLAPI::CreateTexture(int width, int height, TexelType type,
                                       bool isMultiSample, bool doesRepeat, bool mipmap) {
  CheckGLError();
  GLuint texture;
  glGenTextures(1, &texture);
  SetActiveTexture(0);
  CheckGLError();

  if (isMultiSample) {
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
    CheckGLError();
    GLint internalFormat;
    GLenum format, glType;
    GetTextureType(type, internalFormat, format, glType);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 
                            ZENGINE_RENDERTARGET_MULTISAMPLE_COUNT,
                            internalFormat, width, height, false);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);
  } else {
    BindTexture(texture);
    if (type == TexelType::DEPTH32F) {
      //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
      if (mipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8);
      } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      }
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    auto wrapMode = doesRepeat ? GL_REPEAT : GL_CLAMP_TO_EDGE;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    SetTextureData(width, height, type, NULL, mipmap);
  }
  CheckGLError();
  return texture;
}


void OpenGLAPI::SetTextureData(UINT width, UINT height, TexelType type,
                               void* texelData, bool generateMipmap) {
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
  GLint internalFormat;
  GLenum format;
  GLenum glType;
  GetTextureType(type, internalFormat, format, glType);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format, glType, texelData);
  glGenerateMipmap(GL_TEXTURE_2D);
  CheckGLError();
}


void OpenGLAPI::DeleteTexture(TextureHandle handle) {
  glDeleteTextures(1, &handle);
}


void OpenGLAPI::UploadTextureData(TextureHandle handle, int width, int height,
                                  TexelType type, void* texelData) {
  BindTexture(handle);
  SetTextureData(width, height, type, texelData, true);
}


void OpenGLAPI::UploadTextureSubData(TextureHandle handle, UINT x, UINT y,
                                     int width, int height, TexelType type,
                                     void* texelData) {
  BindTexture(handle);
  SetTextureSubData(width, x, y, height, type, texelData);
}


void OpenGLAPI::SetTexture(SamplerId sampler, TextureHandle texture, UINT slotIndex, 
                           bool isRenderTarget) {
  CheckGLError();
  SetActiveTexture(slotIndex);
  if (isRenderTarget) BindMultisampleTexture(texture);
  else BindTexture(texture);
  glUniform1i(sampler, slotIndex);
  CheckGLError();
}


FrameBufferId OpenGLAPI::CreateFrameBuffer(TextureHandle depthBuffer,
                                           TextureHandle targetBufferA,
                                           TextureHandle targetBufferB,
                                           bool isMultiSample) {
  CheckGLError();
  GLuint bufferId;
  glGenFramebuffers(1, &bufferId);
  BindFrameBuffer(bufferId);
  CheckGLError();

  GLenum target = isMultiSample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

  if (depthBuffer) {
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, target, depthBuffer, 0);
  }

  if (!targetBufferA) {
    /// No target buffer
  } else if (!targetBufferB) {
    CheckGLError();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, 
                           targetBufferA, 0);
    GLuint attachments[] = {GL_COLOR_ATTACHMENT0};
    CheckGLError();
    glDrawBuffers(1, attachments);
    CheckGLError();
  } else {
    SHOULD_NOT_HAPPEN;
    GLuint attachments[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);
    CheckGLError();
  }
  CheckGLError();

  if (targetBufferA) {
    NamedFramebufferReadBuffer(bufferId, GL_COLOR_ATTACHMENT0);
    NamedFramebufferDrawBuffer(bufferId, GL_COLOR_ATTACHMENT0);
  }

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE) {
    ERR("Framebuffer incomplete, status: 0x%x\n", status);
  }

  return bufferId;
}


void OpenGLAPI::DeleteFrameBuffer(FrameBufferId frameBufferId) {
  glDeleteFramebuffers(1, &frameBufferId);
}


void OpenGLAPI::SetFrameBuffer(FrameBufferId frameBufferid) {
  BindFrameBuffer(frameBufferid);
  CheckGLError();
}


void OpenGLAPI::BlitFrameBuffer(FrameBufferId source, FrameBufferId target,
                                int srcX0, int srcY0, int srcX1, int srcY1,
                                int dstX0, int dstY0, int dstX1, int dstY1) {
  BindReadFramebuffer(source);
  BindDrawFramebuffer(target);
  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
                    dstX0, dstY0, dstX1, dstY1,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);
  CheckGLError();
}


void OpenGLAPI::SetActiveTexture(GLuint activeTextureIndex) {
  if (ActiveTextureShadow == activeTextureIndex) return;
  glActiveTexture(GL_TEXTURE0 + activeTextureIndex);
  ActiveTextureShadow = activeTextureIndex;
}


void OpenGLAPI::BindTexture(GLuint textureID) {
  if (BoundTextureShadow[ActiveTextureShadow] == textureID) return;
  glBindTexture(GL_TEXTURE_2D, textureID);
  BoundTextureShadow[ActiveTextureShadow] = textureID;
}


void OpenGLAPI::BindMultisampleTexture(GLuint textureID) {
  if (BoundMultisampleTextureShadow[ActiveTextureShadow] == textureID) return;
  glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textureID);
  BoundMultisampleTextureShadow[ActiveTextureShadow] = textureID;
}


void OpenGLAPI::SetVertexBuffer(VertexBufferHandle handle) {
  BindVertexBuffer(handle);
}


void OpenGLAPI::SetIndexBuffer(IndexBufferHandle handle) {
  BindIndexBuffer(handle);
}


void OpenGLAPI::EnableVertexAttribute(UINT index, NodeType nodeType, UINT offset,
                                      UINT stride) {
  GLint size = 0;
  GLenum type = 0;
  switch (nodeType) {
    case NodeType::FLOAT:		size = 1;	type = GL_FLOAT;	break;
    case NodeType::VEC2:		size = 2;	type = GL_FLOAT;	break;
    case NodeType::VEC3:		size = 3;	type = GL_FLOAT;	break;
    case NodeType::VEC4:		size = 4;	type = GL_FLOAT;	break;
    default:
      ERR(L"Unhandled vertex attribute type");
      break;
  }
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)size_t(offset));
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
