#include "stubanalyzer.h"
#include <include/shader/shaderstub.h>

ShaderSource2::ShaderSource2()
	: Node(NodeType::SHADER_SOURCE, "ShaderSource")
	, Stub(NodeType::SHADER_STUB, this, nullptr)
{

}

ShaderSource2::~ShaderSource2()
{}


ShaderStub::ShaderStub(const char* Source)
	: Node(NodeType::SHADER_STUB, "ShaderStub")
{
	ASSERT(Source != nullptr);
	Metadata = StubAnalyzer::FromText(Source);
}

ShaderStub::~ShaderStub()
{}

void ShaderStub::SetStubSource()
{
	
}

const char* ShaderStub::GetStubSource()
{
	NOT_IMPLEMENTED;
	return nullptr;
}

ShaderSource2* ShaderStub::GetShaderSource()
{
	NOT_IMPLEMENTED;
	return nullptr;
}

ShaderStubMetadata* ShaderStub::GetStubMetadata()
{
	return Metadata;
}

ShaderStubMetadata::ShaderStubMetadata(const string& _Name, NodeType _ReturnType,
	const string& _StrippedSource,
	OWNERSHIP const vector<ShaderStubParameter*>& _Parameters,
	const vector<ShaderStubVariable*>& _Inputs,
	const vector<ShaderStubVariable*>& _Outputs,
	const vector<ShaderStubSampler*>& _Samplers)
	: Name(_Name)
	, ReturnType(_ReturnType)
	, Parameters(_Parameters)
	, StrippedSource(_StrippedSource)
	, Inputs(_Inputs)
	, Outputs(_Outputs),
	Samplers(_Samplers)
{}

ShaderStubMetadata::~ShaderStubMetadata()
{
	for (auto x : Parameters) delete(x);
}
