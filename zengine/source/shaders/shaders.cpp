#include <include/shaders/shaders.h>

Globals TheGlobals;

/// Array for global uniform types
static const NodeTypeEnum GlobalUniformTypes[] = {
#undef ITEM
#define ITEM(name, type, token) type,
	GLOBALUSAGE_LIST
};

const int GlobalUniformOffsets[] = {
#undef ITEM
#define ITEM(name, type, token) offsetof(Globals, token),
	GLOBALUSAGE_LIST
};


Shader::Shader( ShaderHandle _Handle, const vector<UniformBlock*>& _UniformBlocks, 
	const vector<Sampler*>& _Samplers, const CompileSettings* _Settings,
	const vector<ShaderAttributeDesc>& _Attributes, const shared_ptr<ShaderSource>& _Source)
	: Handle(_Handle)
	, UniformBlocks(_UniformBlocks)
	, Samplers(_Samplers)
	, Settings(_Settings)
	, Attributes(_Attributes)
	, Source(_Source)
{}


Shader::~Shader()
{
	NOT_IMPLEMENTED;
}


void Shader::Set() const
{
	TheDrawingAPI->SetShaderProgram(Handle);
}

Sampler::Sampler( SamplerId _Handle, const LocalDesc* _Desc )
	: Handle(_Handle)
	, Desc(_Desc)
{}


Uniform::Uniform( NodeTypeEnum _Type, UniformId _Handle, int _ByteOffset, 
	bool _IsLocal )
	: Type(_Type)
	, Handle(_Handle)
	, ByteOffset(_ByteOffset)
	, IsLocal(_IsLocal)
{}


LocalUniform::LocalUniform( NodeTypeEnum _Type, UniformId _Handle, 
	int _ByteOffset, const LocalDesc* _Desc )
	: Uniform(_Type, _Handle, _ByteOffset, true)
	, Desc(_Desc)
{}

GlobalUniform::GlobalUniform( UsageEnum _Usage, UniformId _Handle, int _ByteOffset )
	: Uniform(GlobalUniformTypes[_Usage], _Handle, _ByteOffset, false)
	, Usage(_Usage)
{}

UniformBlock::UniformBlock( const vector<LocalUniform*>& _Locals, 
	const vector<GlobalUniform*>& _Globals, const vector<Uniform*>& _Uniforms, UINT _ByteSize )
	: Locals(_Locals)
	, Globals(_Globals)
	, Uniforms(_Uniforms)
	, ByteSize(_ByteSize)
{}
