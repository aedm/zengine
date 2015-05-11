#pragma once

#include "../dom/node.h"
#include <vector>

using namespace std;

/// Shader parameter, becomes a slot
/// ":param vec4 MyColor"
struct ShaderStubParameter {
	NodeType			Type;
	string				Name;
};

/// Shader variables are outputs of a shader stage and inputs of the next shader stage.
/// ":output vec4 MyColor" -- creates "outMyColor" output variable.
/// ":input vec4 MyColor" -- creates "inMyColor" input variable.
struct ShaderStubVariable {
	NodeType			Type;
	string				Name;
};

/// 2D Texture sampler
/// ":sampler MyTexture"
struct ShaderStubSampler {
	string				Name;
};

/// All metadata collected from a shader stub source.
struct ShaderStubMetadata {
	ShaderStubMetadata(const string& Name, NodeType ReturnType, 
		const string& StrippedSource,
		const vector<OWNERSHIP ShaderStubParameter*>& Parameters,
		const vector<ShaderStubVariable*>& Inputs,
		const vector<ShaderStubVariable*>& Outputs,
		const vector<ShaderStubSampler*>& Samplers);

	~ShaderStubMetadata();

	const string					Name;
	const NodeType					ReturnType;

	const string					StrippedSource;

	const vector<ShaderStubParameter*>	Parameters;
	const vector<ShaderStubVariable*>	Inputs;
	const vector<ShaderStubVariable*>	Outputs;
	const vector<ShaderStubSampler*>	Samplers;
};


///// Shader compile option, like "#define USE_DEPTH_BIAS"
//struct ShaderStubOption
//{
//	ShaderStubOption(const string& Label, const string& Macro);
//
//	/// UI name to display, eg. "Use depth bias"
//	const string				Label;
//
//	/// Shader macro to be defined upon activation, eg. "USE_DEPTH_BIAS"
//	const string				Macro;
//};


class ShaderSource2 : public Node
{
public:
	ShaderSource2();
	virtual ~ShaderSource2();

	Slot				Stub;
};

class ShaderStub : public Node
{
public:
	ShaderStub(const char* Source);
	virtual ~ShaderStub();

	void				SetStubSource();
	const char*			GetStubSource();
	ShaderStubMetadata*	GetStubMetadata();

	ShaderSource2*		GetShaderSource();

protected:
	/// Metadata
	ShaderStubMetadata*	Metadata;

	const char*			StubSource;
	ShaderSource2*		ShaderSrc;
};


