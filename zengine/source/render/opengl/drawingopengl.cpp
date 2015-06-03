
#include "drawingopengl.h"
#include <include/base/helpers.h>
#include <boost/foreach.hpp>
#include <Windows.h>

#ifdef _DEBUG
	void CheckGLError()
	{
		GLenum error = glGetError();
		ASSERT(error == GL_NO_ERROR);
	}
#else
#	define CheckGLError()
#endif

struct GLVersion { GLboolean* Version; const wchar_t* Name; };

static GLVersion OpenGLVersions[] = {
	{ &__GLEW_VERSION_1_1,	 L"1.1"		},
	{ &__GLEW_VERSION_1_1,	 L"1.1"		},	 
	{ &__GLEW_VERSION_1_2,	 L"1.2"		},
	{ &__GLEW_VERSION_1_2_1, L"1.2.1"	},
	{ &__GLEW_VERSION_1_3,	 L"1.3"		},
	{ &__GLEW_VERSION_1_4,	 L"1.4"		},
	{ &__GLEW_VERSION_1_5,	 L"1.5"		},
	{ &__GLEW_VERSION_2_0,	 L"2.0"		},
	{ &__GLEW_VERSION_2_1,	 L"2.1"		},
	{ &__GLEW_VERSION_3_0,	 L"3.0"		},
	{ &__GLEW_VERSION_3_1,	 L"3.1"		},
	{ &__GLEW_VERSION_3_2,	 L"3.2"		},
	{ &__GLEW_VERSION_3_3,	 L"3.3"		},
	{ &__GLEW_VERSION_4_0,	 L"4.0"		},
	{ &__GLEW_VERSION_4_1,	 L"4.1"		},
	{ &__GLEW_VERSION_4_2,	 L"4.2"		},
	{ &__GLEW_VERSION_4_3,	 L"4.3"		},
	{ NULL, NULL }
};

DrawingOpenGL::DrawingOpenGL()
{
	GLenum err = glewInit();
	ASSERT(err == GLEW_OK);
	if (err != GLEW_OK)
	{
		ERR(L"Cannot initialize OpenGL.");
		SHOULDNT_HAPPEN;
	} else {
		const wchar_t* versionName = NULL;
		for (GLVersion* version = OpenGLVersions; version->Version != NULL; version++)
		{
			if (*version->Version) versionName = version->Name;
		}

		if (versionName == NULL) ERR(L"OpenGL not found at all.");
		else INFO(L"OpenGL version %s found.", versionName);

		if(!GLEW_VERSION_2_0)
		{
			ERR(L"Sorry, OpenGL 2.0 needed at least.");
		}
		CheckGLError();
	}

	OnContextSwitch();
}

DrawingOpenGL::~DrawingOpenGL() {}

void DrawingOpenGL::OnContextSwitch()
{
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
	for (int i = 0; i < MAX_COMBINED_TEXTURE_SLOTS; i++)
	{
		BoundTextureShadow[i] = (GLuint)-1;
	}
}

static bool CompileAndAttachShader(GLuint Program, GLuint ShaderType, const char* Source)
{
	/// Create shader object, set the source, and compile
	GLuint shader = glCreateShader(ShaderType);
	GLint length = strlen(Source);
	glShaderSource(shader, 1, (const char **)&Source, &length);
	glCompileShader(shader);

	/// Make sure the compilation was successful
	GLint result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if(result == GL_FALSE) {
		/// Get the shader info log
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		char *log = new char[length];
		glGetShaderInfoLog(shader, length, &result, log);

		ERR(log);

		glDeleteShader(shader);
		return false;
	}
	
	glAttachShader(Program, shader);
	glDeleteShader(shader);
	CheckGLError();
	return true;
}

OWNERSHIP ShaderCompileDesc* DrawingOpenGL::CreateShaderFromSource( const char* VertexSource, const char* FragmentSource )
{
	GLuint program = glCreateProgram();
	
	/// Compile shaders
	if (!CompileAndAttachShader(program, GL_VERTEX_SHADER, VertexSource))
	{
		WARN(L"Vertex shader compilation failed.");
		INFO("\n%s", VertexSource);
		glDeleteProgram(program);
		return NULL;
	}
	if (!CompileAndAttachShader(program, GL_FRAGMENT_SHADER, FragmentSource))
	{
		WARN(L"Fragment shader compilation failed.");
		INFO("\n%s", FragmentSource);
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

	for (int i=0; i<uniformCount; i++)
	{
		/// Request info about Nth uniform
		GLint nameLength;
		GLsizei size;
		GLenum type;
		glGetActiveUniform(program, i, uniformNameMaxLength, &nameLength, &size, &type, name);
		GLint location = glGetUniformLocation(program, name);
		//ASSERT(i == location);
		if (type == GL_SAMPLER_2D)
		{
			ShaderSamplerDesc sampler;
			sampler.Handle = location;
			sampler.Name = name;
			builder->Samplers.push_back(sampler);
		} else {
			ShaderUniformDesc uniform;
			uniform.Handle = location;
			uniform.Name = name;
			switch(type)
			{
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

	for (int i=0; i<attributeCount; i++)
	{
		/// Request info about Nth attribute
		GLint nameLength;
		GLsizei size;
		GLenum type;

		glGetActiveAttrib(program, i, attributeNameMaxLength, &nameLength, &size, &type, name);

		ShaderAttributeDesc attribute;
		attribute.Handle = i;
		attribute.Name = name;
		switch(type)
		{
		case GL_FLOAT:		attribute.Type = NodeType::FLOAT;		break;
		case GL_FLOAT_VEC2:	attribute.Type = NodeType::VEC2;		break;
		case GL_FLOAT_VEC3:	attribute.Type = NodeType::VEC3;		break;
		case GL_FLOAT_VEC4:	attribute.Type = NodeType::VEC4;		break;
		default: NOT_IMPLEMENTED; attribute.Type = (NodeType)-1;	break;
		}

		/// Map attribute name to usage
		bool found = false;
		for (UINT o = 0; o<(UINT)VertexAttributeUsage::COUNT; o++)
		{
			if (strcmp(VertexAttributeName[o], name) == 0)
			{
				attribute.Usage = (VertexAttributeUsage)o;
				found = true;
				break;
			}
		}
		if (!found)
		{
			ERR("Unrecognized vertex attribute name: %s", name);
		}

		builder->Attributes.push_back(attribute);
	}

	CheckGLError();
	return builder;
}

void DrawingOpenGL::SetShaderProgram( ShaderHandle Handle )
{
	glUseProgram(Handle);
	CheckGLError();
}

void DrawingOpenGL::SetUniform( UniformId Id, NodeType Type, const void* Values )
{
	CheckGLError();

	switch(Type)
	{
	case NodeType::FLOAT:		glUniform1f(Id, *(const GLfloat*)Values);					break;
	case NodeType::VEC2:		glUniform2fv(Id, 1, (const GLfloat*)Values);				break;
	case NodeType::VEC3:		glUniform3fv(Id, 1, (const GLfloat*)Values);				break;
	case NodeType::VEC4:		glUniform4fv(Id, 1, (const GLfloat*)Values);				break;
	case NodeType::MATRIX44:	glUniformMatrix4fv(Id, 1, false, (const GLfloat*)Values);	break;
	
	default: NOT_IMPLEMENTED; break;
	}

	CheckGLError();
}

void DrawingOpenGL::DestroyShaderProgram( ShaderHandle Handle )
{
	glDeleteProgram(Handle);
	CheckGLError();
}

VertexBufferHandle DrawingOpenGL::CreateVertexBuffer( UINT Size )
{
	VertexBufferHandle handle;
	glGenBuffers(1, &handle);
	CheckGLError();
	BindVertexBuffer(handle);
	glBufferData(GL_ARRAY_BUFFER, Size, NULL, GL_STATIC_DRAW);
	CheckGLError();
	return handle;
}

void DrawingOpenGL::DestroyVertexBuffer( VertexBufferHandle Handle )
{
	glDeleteBuffers(1, &Handle);
}

void* DrawingOpenGL::MapVertexBuffer( VertexBufferHandle Handle )
{
	BindVertexBuffer(Handle);
	void* address = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	CheckGLError();
	return address;
}

void DrawingOpenGL::UnMapVertexBuffer( VertexBufferHandle Handle )
{
	/// Please don't do anything else while a buffer is mapped
	ASSERT(BoundVertexBufferShadow == Handle);

	glUnmapBuffer(GL_ARRAY_BUFFER);
}

IndexBufferHandle DrawingOpenGL::CreateIndexBuffer( UINT Size )
{
	IndexBufferHandle handle;
	glGenBuffers(1, &handle);
	CheckGLError();
	BindIndexBuffer(handle);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, Size, NULL, GL_STATIC_DRAW);
	CheckGLError();
	return handle;
}

void DrawingOpenGL::DestroyIndexBuffer( IndexBufferHandle Handle )
{
	glDeleteBuffers(1, &Handle);
}

void* DrawingOpenGL::MapIndexBuffer( IndexBufferHandle Handle )
{
	BindIndexBuffer(Handle);
	void* buffer = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	CheckGLError();
	return buffer;
}

void DrawingOpenGL::UnMapIndexBuffer( IndexBufferHandle Handle )
{
	/// Please don't do anything else while a buffer is mapped
	ASSERT(BoundIndexBufferShadow == Handle);

	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void DrawingOpenGL::BindVertexBuffer( GLuint BufferID )
{
	if (BufferID != BoundVertexBufferShadow)
	{
		glBindBuffer(GL_ARRAY_BUFFER, BufferID);
		BoundVertexBufferShadow = BufferID;
		CheckGLError();
	}
}

void DrawingOpenGL::BindIndexBuffer( GLuint BufferID )
{
	if (BufferID != BoundIndexBufferShadow)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BufferID);
		BoundIndexBufferShadow = BufferID;
		CheckGLError();
	}
}


AttributeMapper* DrawingOpenGL::CreateAttributeMapper( const vector<VertexAttribute>& BufferAttribs, 
	const vector<ShaderAttributeDesc>& ShaderAttribs, UINT Stride )
{
	AttributeMapperOpenGL* mapper = new AttributeMapperOpenGL();
	mapper->Stride = Stride;

	foreach(const ShaderAttributeDesc& shaderAttr, ShaderAttribs)
	{
		bool found = false;
		foreach(const VertexAttribute& bufferAttr, BufferAttribs)
		{
			if (bufferAttr.Usage == shaderAttr.Usage)
			{
				MappedAttributeOpenGL attr;
				attr.Index = shaderAttr.Handle;
				switch(VertexAttributeType[(UINT)bufferAttr.Usage])
				{
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
		if (!found)
		{
			ERR(L"Shader needs a vertex attribute that's missing from the buffer.");
		}
	}
	return mapper;
}

GLenum GetGLPrimitive(PrimitiveTypeEnum PrimitiveType)
{
	switch(PrimitiveType)
	{
	case PRIMITIVE_LINES:		return GL_LINES;
	case PRIMITIVE_LINE_STRIP:	return GL_LINE_STRIP;
	case PRIMITIVE_TRIANGLES:	return GL_TRIANGLES;
	}
	SHOULDNT_HAPPEN;
	return 0;
}

void DrawingOpenGL::RenderIndexedMesh( IndexBufferHandle IndexHandle, 
	UINT IndexCount, VertexBufferHandle VertexHandle, const AttributeMapper* Mapper,
	PrimitiveTypeEnum PrimitiveType)
{
	BindVertexBuffer(VertexHandle);
	static_cast<const AttributeMapperOpenGL*>(Mapper)->Set();

	BindIndexBuffer(IndexHandle);
	glDrawElements(GetGLPrimitive(PrimitiveType), IndexCount, GL_UNSIGNED_INT, NULL);
	CheckGLError();
}

void DrawingOpenGL::RenderMesh( VertexBufferHandle VertexHandle, UINT VertexCount, 
	const AttributeMapper* Mapper, PrimitiveTypeEnum PrimitiveType )
{
	BindVertexBuffer(VertexHandle);
	static_cast<const AttributeMapperOpenGL*>(Mapper)->Set();
	glDrawArrays(GetGLPrimitive(PrimitiveType), 0, VertexCount);
	CheckGLError();
}

void DrawingOpenGL::Render(IndexBufferHandle IndexBuffer, UINT Count, PrimitiveTypeEnum PrimitiveType)
{
	if (IndexBuffer != 0)
	{
		BindIndexBuffer(IndexBuffer);
		glDrawElements(GetGLPrimitive(PrimitiveType), Count, GL_UNSIGNED_INT, NULL);
		CheckGLError();
	} else {
		glDrawArrays(GetGLPrimitive(PrimitiveType), 0, Count);
	}
}

void DrawingOpenGL::SetViewport( int X, int Y, int Width, int Height, float DepthMin /*= 0.0f*/, float DepthMax /*= 1.0f*/ )
{
	glViewport(X, Y, Width, Height);
	glDepthRange(DepthMin, DepthMax);
}

void DrawingOpenGL::Clear( bool ColorBuffer, bool DepthBuffer, UINT RGBColor /*= 0*/)
{
	SetClearColor(RGBColor);
	glClear((ColorBuffer ? GL_COLOR_BUFFER_BIT : 0) | (DepthBuffer ? GL_DEPTH_BUFFER_BIT : 0));
}

void DrawingOpenGL::SetRenderState( const RenderState* State )
{
	SetDepthTest(State->DepthTest);
	SetFace(State->Face);
	SetBlendMode(State->BlendMode);
}

void DrawingOpenGL::SetDepthTest( bool Enable )
{
	if (Enable == DepthTestEnabledShadow) return;
	if (Enable) glEnable(GL_DEPTH_TEST);
	else glDisable(GL_DEPTH_TEST);
	DepthTestEnabledShadow = Enable;
}

void DrawingOpenGL::SetBlendMode( RenderState::BlendModeEnum BlendMode )
{
	if (BlendMode == BlendModeShadow) return;
	switch(BlendMode)
	{
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
	BlendModeShadow = BlendMode;
}

void DrawingOpenGL::SetBlending( bool Enable )
{
	if (Enable == BlendEnableShadow) return;
	if (Enable) glEnable(GL_BLEND);
	else glDisable(GL_BLEND);
	BlendEnableShadow = Enable;
}

void DrawingOpenGL::SetFace( RenderState::FaceEnum Face )
{
	if (FaceShadow == Face) return;
	switch(Face)
	{
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
	FaceShadow = Face;
}

inline float IntColorToFloat(UINT Color)
{
	return float(Color) / 255.0f;
}

void DrawingOpenGL::SetClearColor( UINT ClearColor )
{
	if (ClearColor == ClearColorShadow) return;
	glClearColor(
		IntColorToFloat((ClearColor >> 16) & 0xff), 
		IntColorToFloat((ClearColor >> 8 ) & 0xff), 
		IntColorToFloat((ClearColor      ) & 0xff),
		1.0f);
	ClearColorShadow = ClearColor;
}

TextureHandle DrawingOpenGL::CreateTexture( int Width, int Height, TexelTypeEnum Type )
{
	GLuint texture;
	glGenTextures(1, &texture);
	BindTexture(texture);

	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

	SetTextureData(Width, Height, Type, NULL);
	/// TODO: mipmap
	
	return texture;
}

/// Converts a single TexelTypeEnum to OpenGL enums
void GetTextureType( TexelTypeEnum Type, GLint &internalFormat, GLenum &format, GLenum &glType ) 
{
	switch (Type)
	{
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

void DrawingOpenGL::SetTextureData( UINT Width, UINT Height, TexelTypeEnum Type, void* TexelData )
{
	GLint internalFormat;
	GLenum format;
	GLenum glType;
	GetTextureType(Type, internalFormat, format, glType);
	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, Width, Height, 0, format, glType, TexelData);
	glGenerateMipmap( GL_TEXTURE_2D );
	CheckGLError();
}

void DrawingOpenGL::SetTextureSubData( UINT X, UINT Y, UINT Width, UINT Height, TexelTypeEnum Type, void* TexelData )
{
	GLint internalFormat;
	GLenum format;
	GLenum glType;
	GetTextureType(Type, internalFormat, format, glType);
	glTexSubImage2D( GL_TEXTURE_2D, 0, X, Y, Width, Height, format, glType, TexelData);
	glGenerateMipmap( GL_TEXTURE_2D );
	CheckGLError();
}


void DrawingOpenGL::DeleteTexture( TextureHandle Handle )
{
	glDeleteTextures(1, &Handle);
}

void DrawingOpenGL::UploadTextureData( TextureHandle Handle, int Width, int Height, TexelTypeEnum Type, void* TexelData )
{
	BindTexture(Handle);
	SetTextureData(Width, Height, Type, TexelData);
}

void DrawingOpenGL::UploadTextureSubData( TextureHandle Handle, UINT X, UINT Y, int Width, int Height, TexelTypeEnum Type, void* TexelData )
{
	BindTexture(Handle);
	SetTextureSubData(Width, X, Y, Height, Type, TexelData);
}

void DrawingOpenGL::SetTexture( SamplerId Sampler, TextureHandle Texture, UINT SlotIndex )
{
	SetActiveTexture(SlotIndex);
	BindTexture(Texture);
	glUniform1i(Sampler, SlotIndex);
}

void DrawingOpenGL::SetActiveTexture( GLuint ActiveTextureIndex )
{
	if (ActiveTextureShadow == ActiveTextureIndex) return;
	glActiveTexture(GL_TEXTURE0 + ActiveTextureIndex);
	ActiveTextureShadow = ActiveTextureIndex;
}

void DrawingOpenGL::BindTexture( GLuint TextureID )
{
	if (BoundTextureShadow[ActiveTextureShadow] == TextureID) return;
	glBindTexture(GL_TEXTURE_2D, TextureID);
	BoundTextureShadow[ActiveTextureShadow] = TextureID;
}

void DrawingOpenGL::SetVertexBuffer(VertexBufferHandle Handle)
{
	BindVertexBuffer(Handle);
}

void DrawingOpenGL::SetIndexBuffer(IndexBufferHandle Handle)
{
	BindIndexBuffer(Handle);
}

void DrawingOpenGL::EnableVertexAttribute(UINT Index, NodeType Type, UINT Offset, UINT Stride)
{
	GLint size = 0;
	GLenum type = 0;
	switch (Type)
	{
	case NodeType::FLOAT:		size = 1;	type = GL_FLOAT;	break;
	case NodeType::VEC2:		size = 2;	type = GL_FLOAT;	break;
	case NodeType::VEC3:		size = 3;	type = GL_FLOAT;	break;
	case NodeType::VEC4:		size = 4;	type = GL_FLOAT;	break;
	default:
		ERR(L"Unhandled vertex attribute type");
		break;
	}
	glEnableVertexAttribArray(Index);
	glVertexAttribPointer(Index, size, type, GL_FALSE, Stride, (void*)Offset);
}

AttributeMapperOpenGL::AttributeMapperOpenGL()
{
	Stride = 0;
}

void AttributeMapperOpenGL::Set() const
{
	foreach(const MappedAttributeOpenGL& attr, MappedAttributes)
	{
		glEnableVertexAttribArray(attr.Index);
		glVertexAttribPointer(attr.Index, attr.Size, attr.Type, GL_FALSE, Stride, (void*)attr.Offset);
		CheckGLError();
	}
}
