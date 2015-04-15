#include "shadertokenizer.h"
#include "analyzer.h"
#include <include/shaders/shadermetadata.h>
#include <include/shaders/shaderbuilder.h>
#include <include/base/defines.h>
#include <include/base/helpers.h>
#include <include/render/drawingapi.h>

#include <boost/foreach.hpp>
#include <ctime>

OWNERSHIP ShaderMetadata* ShaderBuilder::FromText(const char* FileName, const char* Source )
{
	Shaders::Analysis analysis;
	return analysis.FromText(FileName, Source);
}

OWNERSHIP ShaderMetadata* ShaderBuilder::Merge( const vector<ShaderMetadata*>& MetadataList )
{
	vector<ShaderOption*> options;
	vector<ShaderChoice*> choices;
	vector<LocalDesc*> locals;

	foreach (ShaderMetadata* metadata, MetadataList)
	{
		foreach (ShaderOption* option, metadata->Options) options.push_back(option);
		foreach (ShaderChoice* choice, metadata->Choices) choices.push_back(choice);
		foreach (LocalDesc* local, metadata->Locals) locals.push_back(local);
	}

	return new ShaderMetadata(options, choices, locals);
}

ShaderSource* ShaderSource::FromSource( OWNERSHIP const char* VertexSource, 
	OWNERSHIP const char* FragmentSource)
{
	ShaderSource* source = new ShaderSource;

	source->VertexSource = VertexSource;
	source->FragmentSource = FragmentSource;
	
	std::clock_t start = std::clock();

	source->VertexMetadata = ShaderBuilder::FromText("vertex", VertexSource);
	source->FragmentMetadata = ShaderBuilder::FromText("fragment", FragmentSource);

	/// Check for identical uniform names
	foreach (LocalDesc* vertexLocal, source->VertexMetadata->Locals)
	{
		foreach (LocalDesc* fragmentLocal, source->FragmentMetadata->Locals)
		{
			if (vertexLocal->UniformName == fragmentLocal->UniformName)
			{
				ERR("Uniform with identical name '%s' in vertex and fragment shader.", 
					vertexLocal->UniformName->c_str());
			}
		}
	}

	vector<ShaderMetadata*> metas;
	metas.push_back(source->VertexMetadata);
	metas.push_back(source->FragmentMetadata);
	source->Metadata = ShaderBuilder::Merge(metas);
	INFO("Shaders analyzed in %d ms.", 
		int(float(std::clock()-start) / (float(CLOCKS_PER_SEC)/1000.0f)));

	return source;
}

ShaderSource* ShaderSource::FromFile( const wchar_t* VertexShaderFile, 
	const wchar_t* FragmentShaderFile )
{
	char* vertexSrc = System::ReadFile(VertexShaderFile);
	if (vertexSrc == NULL)
	{
		ERR("Cannot load vertex shader: %s", VertexShaderFile);
		return NULL;
	}
	char* fragmentSrc = System::ReadFile(FragmentShaderFile);
	if (fragmentSrc == NULL)
	{
		delete vertexSrc;
		ERR("Cannot load fragment shader: %s", FragmentShaderFile);
		return NULL;
	}

	return FromSource(vertexSrc, fragmentSrc);
}

ShaderSource::~ShaderSource()
{
	SafeDelete(VertexMetadata);
	SafeDelete(FragmentMetadata);
	Metadata = NULL; /// It's a merged object
	SafeDelete(VertexSource);
	SafeDelete(FragmentSource);
}

OWNERSHIP Shader* ShaderSource::Build(const shared_ptr<ShaderSource>& Source)
{
	std::clock_t start = std::clock();

	ShaderCompileDesc* shaderCompileDesc = 
		TheDrawingAPI->CreateShaderFromSource(Source->VertexSource, Source->FragmentSource);
	if (shaderCompileDesc == NULL)
	{
		return NULL;
	}

	/// Process uniforms. Currently only one uniform block is supported,
	/// the global namespace itself.
	vector<LocalUniform*> locals;
	vector<GlobalUniform*> globals;
	vector<Uniform*> uniforms;
	UINT byteOffset = 0;

	foreach(ShaderUniformDesc& uniformDesc, shaderCompileDesc->Uniforms)
	{
		Uniform* uniform = NULL;
		if (uniformDesc.Name[0] != 'g') 
		{
			//ASSERT(uniformDesc.UniformType == OP_FLOAT);

			/// Get metadata for local
			LocalDesc* localDesc = Source->GetLocalDesc(uniformDesc.Name);
			if (localDesc)
			{
				LocalUniform* local = new LocalUniform(uniformDesc.UniformType, 
					uniformDesc.Handle, byteOffset, localDesc);
				locals.push_back(local);
				uniform = local;
			} else {
				ERR("Unhandled uniform '%s'. Please define one uniform per line.", 
					uniformDesc.Name.c_str());
			}
			
		} else {
			int usage = EnumMapperA::GetEnumFromString(GlobalUniformMapper, 
				uniformDesc.Name.c_str());
			if (usage >= 0)
			{
				GlobalUniform* global = new GlobalUniform((GlobalUniform::UsageEnum)usage, 
					uniformDesc.Handle, byteOffset);
				globals.push_back(global);
				uniform = global;
			}
			else uniform = NULL;
		}

		if (uniform)
		{
			byteOffset += VariableByteSizes[UINT(uniform->Type)];
			uniforms.push_back(uniform);
		}
	}

	/// Create a single uniform block
	vector<UniformBlock*> uniformBlocks;
	UniformBlock* globalBlock = 
		new UniformBlock(locals, globals, uniforms, byteOffset);
	uniformBlocks.push_back(globalBlock);

	/// Process samplers
	vector<Sampler*> samplers;
	foreach(ShaderSamplerDesc& samplerDesc, shaderCompileDesc->Samplers)
	{
		/// All samplers are locals now, get metadata
		LocalDesc* localDesc = Source->GetLocalDesc(samplerDesc.Name);
		Sampler* sampler = new Sampler(samplerDesc.Handle, localDesc);
		samplers.push_back(sampler);
	}

	Shader* program = new Shader(shaderCompileDesc->Handle, uniformBlocks, 
		samplers, NULL, shaderCompileDesc->Attributes, Source);

	INFO("Shader built in %d ms.", int(float(std::clock()-start) / (float(CLOCKS_PER_SEC)/1000.0f)));

	return program;
}

ShaderSource::ShaderSource()
	: VertexSource(NULL)
	, FragmentSource(NULL)
	, VertexMetadata(NULL)
	, FragmentMetadata(NULL)
	, Metadata(NULL)
{}


LocalDesc* ShaderSource::GetLocalDesc( const string& UniformName )
{
	foreach(LocalDesc* localDesc, Metadata->Locals)
	{
		if (*localDesc->UniformName == UniformName) return localDesc;
	}
	return NULL;
}
