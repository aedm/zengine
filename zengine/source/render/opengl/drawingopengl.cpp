#include "drawingopengl.h"
#include <include/base/helpers.h>
#include <boost/foreach.hpp>
#include <Windows.h>

#ifdef _DEBUG
void CheckGLError() {
  GLenum error = glGetError();
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
  {NULL, NULL}
};

DrawingOpenGL::DrawingOpenGL() {
  GLenum err = glewInit();
  ASSERT(err == GLEW_OK);
  if (err != GLEW_OK) {
    ERR(L"Cannot initialize OpenGL.");
    SHOULDNT_HAPPEN;
  } else {
    const wchar_t* versionName = NULL;
    for (GLVersion* version = gOpenGLVersions; version->version != NULL; version++) {
      if (*version->version) versionName = version->name;
    }

    if (versionName == NULL) ERR(L"OpenGL not found at all.");
    else INFO(L"OpenGL version %s found.", versionName);

    if (!GLEW_VERSION_2_0) {
      ERR(L"Sorry, OpenGL 2.0 needed at least.");
    }
    CheckGLError();
  }

  OnContextSwitch();
}

DrawingOpenGL::~DrawingOpenGL() {}

void DrawingOpenGL::OnContextSwitch() {
  /// Set defaults (shadow values must be something different at the beginning to avoid false cache hit)
  FaceShadow = RenderState::FACE_BACK;
  SetFace(RenderState::FACE_FRONT_AND_BACK);

  BlendModeShadow = RenderState::BLEND_ADDITIVE;
  BlendEnableShadow = true;
  SetBlendMode(RenderState::BLEND_NORMAL);

  DepthTestEnabledShadow = true;
  SetDepthTest(false);

  ClearColorShadow = 1;
  SetClearColor(0);

  glDepthMask(true);

  ActiveTextureShadow = 0;
  BoundVertexBufferShadow = 0;
  BoundIndexBufferShadow = 0;
  for (int i = 0; i < MAX_COMBINED_TEXTURE_SLOTS; i++) {
    BoundTextureShadow[i] = (GLuint)-1;
  }
}

static bool CompileAndAttachShader(GLuint program, GLuint shaderType, 
                                   const char* source) {
  /// Create shader object, set the source, and compile
  GLuint shader = glCreateShader(shaderType);
  GLint length = strlen(source);
  glShaderSource(shader, 1, (const char **)&source, &length);
  glCompileShader(shader);

  /// Make sure the compilation was successful
  GLint result;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
  if (result == GL_FALSE) {
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

OWNERSHIP ShaderCompileDesc* DrawingOpenGL::CreateShaderFromSource(
    const char* vertexSource, const char* fragmentSource) {
  GLuint program = glCreateProgram();

  /// Compile shaders
  if (!CompileAndAttachShader(program, GL_VERTEX_SHADER, vertexSource)) {
    WARN(L"Vertex shader compilation failed.");
    INFO("\n%s", vertexSource);
    glDeleteProgram(program);
    return NULL;
  }
  if (!CompileAndAttachShader(program, GL_FRAGMENT_SHADER, fragmentSource)) {
    WARN(L"Fragment shader compilation failed.");
    INFO("\n%s", fragmentSource);
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
    if (type == GL_SAMPLER_2D) {
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

    ShaderAttributeDesc attribute;
    attribute.Handle = i;
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

void DrawingOpenGL::SetShaderProgram(ShaderHandle handle) {
  glUseProgram(handle);
  CheckGLError();
}

void DrawingOpenGL::SetUniform(UniformId id, NodeType type, const void* values) {
  CheckGLError();

  switch (type) {
    case NodeType::FLOAT:		glUniform1f(id, *(const GLfloat*)values);					break;
    case NodeType::VEC2:		glUniform2fv(id, 1, (const GLfloat*)values);				break;
    case NodeType::VEC3:		glUniform3fv(id, 1, (const GLfloat*)values);				break;
    case NodeType::VEC4:		glUniform4fv(id, 1, (const GLfloat*)values);				break;
    case NodeType::MATRIX44:	glUniformMatrix4fv(id, 1, false, (const GLfloat*)values);	break;

    default: NOT_IMPLEMENTED; break;
  }

  CheckGLError();
}

void DrawingOpenGL::DestroyShaderProgram(ShaderHandle handle) {
  glDeleteProgram(handle);
  CheckGLError();
}

VertexBufferHandle DrawingOpenGL::CreateVertexBuffer(UINT size) {
  VertexBufferHandle handle;
  glGenBuffers(1, &handle);
  CheckGLError();
  BindVertexBuffer(handle);
  glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
  CheckGLError();
  return handle;
}

void DrawingOpenGL::DestroyVertexBuffer(VertexBufferHandle handle) {
  glDeleteBuffers(1, &handle);
}

void* DrawingOpenGL::MapVertexBuffer(VertexBufferHandle handle) {
  BindVertexBuffer(handle);
  void* address = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  CheckGLError();
  return address;
}

void DrawingOpenGL::UnMapVertexBuffer(VertexBufferHandle handle) {
  /// Please don't do anything else while a buffer is mapped
  ASSERT(BoundVertexBufferShadow == handle);

  glUnmapBuffer(GL_ARRAY_BUFFER);
}

IndexBufferHandle DrawingOpenGL::CreateIndexBuffer(UINT size) {
  IndexBufferHandle handle;
  glGenBuffers(1, &handle);
  CheckGLError();
  BindIndexBuffer(handle);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
  CheckGLError();
  return handle;
}

void DrawingOpenGL::DestroyIndexBuffer(IndexBufferHandle handle) {
  glDeleteBuffers(1, &handle);
}

void* DrawingOpenGL::MapIndexBuffer(IndexBufferHandle handle) {
  BindIndexBuffer(handle);
  void* buffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
  CheckGLError();
  return buffer;
}

void DrawingOpenGL::UnMapIndexBuffer(IndexBufferHandle handle) {
  /// Please don't do anything else while a buffer is mapped
  ASSERT(BoundIndexBufferShadow == handle);

  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void DrawingOpenGL::BindVertexBuffer(GLuint bufferID) {
  if (bufferID != BoundVertexBufferShadow) {
    glBindBuffer(GL_ARRAY_BUFFER, bufferID);
    BoundVertexBufferShadow = bufferID;
    CheckGLError();
  }
}

void DrawingOpenGL::BindIndexBuffer(GLuint bufferID) {
  if (bufferID != BoundIndexBufferShadow) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferID);
    BoundIndexBufferShadow = bufferID;
    CheckGLError();
  }
}


AttributeMapper* DrawingOpenGL::CreateAttributeMapper(
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
  SHOULDNT_HAPPEN;
  return 0;
}

void DrawingOpenGL::RenderIndexedMesh(IndexBufferHandle indexHandle,
                                      UINT indexCount, VertexBufferHandle vertexHandle, 
                                      const AttributeMapper* mapper,
                                      PrimitiveTypeEnum primitiveType) {
  BindVertexBuffer(vertexHandle);
  static_cast<const AttributeMapperOpenGL*>(mapper)->Set();

  BindIndexBuffer(indexHandle);
  glDrawElements(GetGLPrimitive(primitiveType), indexCount, GL_UNSIGNED_INT, NULL);
  CheckGLError();
}

void DrawingOpenGL::RenderMesh(VertexBufferHandle vertexHandle, UINT vertexCount,
                               const AttributeMapper* mapper, 
                               PrimitiveTypeEnum primitiveType) {
  BindVertexBuffer(vertexHandle);
  static_cast<const AttributeMapperOpenGL*>(mapper)->Set();
  glDrawArrays(GetGLPrimitive(primitiveType), 0, vertexCount);
  CheckGLError();
}

void DrawingOpenGL::Render(IndexBufferHandle indexBuffer, UINT count, 
                           PrimitiveTypeEnum primitiveType) {
  if (indexBuffer != 0) {
    BindIndexBuffer(indexBuffer);
    CheckGLError();
    glDrawElements(GetGLPrimitive(primitiveType), count, GL_UNSIGNED_INT, NULL);
    CheckGLError();
  } else {
    glDrawArrays(GetGLPrimitive(primitiveType), 0, count);
  }
}

void DrawingOpenGL::SetViewport(int x, int y, int width, int height, 
                                float depthMin /*= 0.0f*/, float depthMax /*= 1.0f*/) {
  glViewport(x, y, width, height);
  glDepthRange(depthMin, depthMax);
}

void DrawingOpenGL::Clear(bool colorBuffer, bool depthBuffer, UINT rgbColor /*= 0*/) {
  SetClearColor(rgbColor);
  glClear((colorBuffer ? GL_COLOR_BUFFER_BIT : 0) | 
          (depthBuffer ? GL_DEPTH_BUFFER_BIT : 0));
}

void DrawingOpenGL::SetRenderState(const RenderState* state) {
  SetDepthTest(state->DepthTest);
  SetFace(state->Face);
  SetBlendMode(state->BlendMode);
}

void DrawingOpenGL::SetDepthTest(bool enable) {
  if (enable == DepthTestEnabledShadow) return;
  if (enable) glEnable(GL_DEPTH_TEST);
  else glDisable(GL_DEPTH_TEST);
  DepthTestEnabledShadow = enable;
}

void DrawingOpenGL::SetBlendMode(RenderState::BlendModeEnum blendMode) {
  if (blendMode == BlendModeShadow) return;
  switch (blendMode) {
    case RenderState::BLEND_NORMAL:
      SetBlending(false);
      break;
    case RenderState::BLEND_ADDITIVE:
      SetBlending(true);
      glBlendFunc(GL_ONE, GL_ONE);
      break;
    case RenderState::BLEND_ALPHA:
      SetBlending(true);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      break;
  }
  BlendModeShadow = blendMode;
}

void DrawingOpenGL::SetBlending(bool enable) {
  if (enable == BlendEnableShadow) return;
  if (enable) glEnable(GL_BLEND);
  else glDisable(GL_BLEND);
  BlendEnableShadow = enable;
}

void DrawingOpenGL::SetFace(RenderState::FaceEnum face) {
  if (FaceShadow == face) return;
  switch (face) {
    case RenderState::FACE_FRONT:
      glPolygonMode(GL_FRONT, GL_FILL);
      break;
    case RenderState::FACE_FRONT_AND_BACK:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;
    case RenderState::FACE_BACK:
      glPolygonMode(GL_BACK, GL_FILL);
      break;
  }
  FaceShadow = face;
}

inline float IntColorToFloat(UINT color) {
  return float(color) / 255.0f;
}

void DrawingOpenGL::SetClearColor(UINT clearColor) {
  if (clearColor == ClearColorShadow) return;
  glClearColor(
    IntColorToFloat((clearColor >> 16) & 0xff),
    IntColorToFloat((clearColor >> 8) & 0xff),
    IntColorToFloat((clearColor)& 0xff),
    1.0f);
  ClearColorShadow = clearColor;
}

TextureHandle DrawingOpenGL::CreateTexture(int width, int height, TexelTypeEnum type) {
  GLuint texture;
  glGenTextures(1, &texture);
  BindTexture(texture);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  SetTextureData(width, height, type, NULL);
  /// TODO: mipmap

  return texture;
}

/// Converts a single TexelTypeEnum to OpenGL enums
void GetTextureType(TexelTypeEnum type, GLint &internalFormat, GLenum &format, 
                    GLenum &glType) {
  switch (type) {
    case TEXELTYPE_RGBA_UINT8:
      internalFormat = GL_RGBA;
      format = GL_RGBA;
      glType = GL_UNSIGNED_BYTE;
      break;
    case TEXELTYPE_RGBA_UINT16:
      internalFormat = GL_RGBA16;
      format = GL_RGBA;
      glType = GL_UNSIGNED_SHORT;
      break;
    case TEXELTYPE_RGBA_FLOAT16:
      internalFormat = GL_RGBA16F;
      format = GL_RGBA;
      glType = GL_FLOAT;
      break;
    case TEXELTYPE_RGBA_FLOAT32:
      internalFormat = GL_RGBA32F;
      format = GL_RGBA;
      glType = GL_FLOAT;
      break;
    default:
      NOT_IMPLEMENTED;
      break;
  }
}

void DrawingOpenGL::SetTextureData(UINT width, UINT height, TexelTypeEnum type, 
                                   void* texelData) {
  GLint internalFormat;
  GLenum format;
  GLenum glType;
  GetTextureType(type, internalFormat, format, glType);
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, glType, texelData);
  glGenerateMipmap(GL_TEXTURE_2D);
  CheckGLError();
}

void DrawingOpenGL::SetTextureSubData(UINT x, UINT y, UINT width, UINT height, 
                                      TexelTypeEnum type, void* texelData) {
  GLint internalFormat;
  GLenum format;
  GLenum glType;
  GetTextureType(type, internalFormat, format, glType);
  glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, width, height, format, glType, texelData);
  glGenerateMipmap(GL_TEXTURE_2D);
  CheckGLError();
}


void DrawingOpenGL::DeleteTexture(TextureHandle handle) {
  glDeleteTextures(1, &handle);
}

void DrawingOpenGL::UploadTextureData(TextureHandle handle, int width, int height, 
                                      TexelTypeEnum type, void* texelData) {
  BindTexture(handle);
  SetTextureData(width, height, type, texelData);
}

void DrawingOpenGL::UploadTextureSubData(TextureHandle handle, UINT x, UINT y, 
                                         int width, int height, TexelTypeEnum type, 
                                         void* texelData) {
  BindTexture(handle);
  SetTextureSubData(width, x, y, height, type, texelData);
}

void DrawingOpenGL::SetTexture(SamplerId sampler, TextureHandle texture, UINT slotIndex) {
  SetActiveTexture(slotIndex);
  BindTexture(texture);
  glUniform1i(sampler, slotIndex);
}

void DrawingOpenGL::SetActiveTexture(GLuint activeTextureIndex) {
  if (ActiveTextureShadow == activeTextureIndex) return;
  glActiveTexture(GL_TEXTURE0 + activeTextureIndex);
  ActiveTextureShadow = activeTextureIndex;
}

void DrawingOpenGL::BindTexture(GLuint textureID) {
  if (BoundTextureShadow[ActiveTextureShadow] == textureID) return;
  glBindTexture(GL_TEXTURE_2D, textureID);
  BoundTextureShadow[ActiveTextureShadow] = textureID;
}

void DrawingOpenGL::SetVertexBuffer(VertexBufferHandle handle) {
  BindVertexBuffer(handle);
}

void DrawingOpenGL::SetIndexBuffer(IndexBufferHandle handle) {
  BindIndexBuffer(handle);
}

void DrawingOpenGL::EnableVertexAttribute(UINT index, NodeType nodeType, UINT offset, 
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
  glVertexAttribPointer(index, size, type, GL_FALSE, stride, (void*)offset);
}

AttributeMapperOpenGL::AttributeMapperOpenGL() {
  Stride = 0;
}

void AttributeMapperOpenGL::Set() const {
  for (const MappedAttributeOpenGL& attr : MappedAttributes) {
    glEnableVertexAttribArray(attr.Index);
    glVertexAttribPointer(attr.Index, attr.Size, attr.Type, GL_FALSE, Stride, (void*)attr.Offset);
    CheckGLError();
  }
}
