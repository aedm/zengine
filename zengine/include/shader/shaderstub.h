#pragma once

#include "../dom/node.h"
#include "../shaders/shaders.h"
#include <vector>
#include <map>

using namespace std;
class ShaderSource2;

enum class ShaderGlobalType
{
#undef ITEM
#define ITEM(name, type, variable) name,
	GLOBALUSAGE_LIST

	LOCAL,	/// For non-global uniforms
};

/// Shader parameter, becomes a slot
/// ":param vec4 MyColor" or ":param sampler2d MyTexture"
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

/// --
struct ShaderStubGlobal {
	NodeType			Type;
	string				Name;
	ShaderGlobalType	Usage;
};


/// All metadata collected from a shader stub source.
struct ShaderStubMetadata {
	ShaderStubMetadata(const string& Name, NodeType ReturnType, 
		const string& StrippedSource,
		const vector<OWNERSHIP ShaderStubParameter*>& Parameters,
		const vector<ShaderStubGlobal*>& Globals,
		const vector<ShaderStubVariable*>& Inputs,
		const vector<ShaderStubVariable*>& Outputs);

	~ShaderStubMetadata();

	const string					Name;
	const NodeType					ReturnType;

	const string					StrippedSource;

	const vector<ShaderStubParameter*>	Parameters;
	const vector<ShaderStubGlobal*>		Globals;
	const vector<ShaderStubVariable*>	Inputs;
	const vector<ShaderStubVariable*>	Outputs;
};


class ShaderStub : public Node
{
public:
	ShaderStub(const string& Source);
	ShaderStub(const ShaderStub& Original);
	virtual ~ShaderStub();

	virtual Node*					Clone() const;

	void							SetStubSource(const string& Source);
	const string&					GetStubSource() const;
	ShaderStubMetadata*				GetStubMetadata() const;

	ShaderSource2*					GetShaderSource();
	const map<ShaderStubParameter*, Slot*>& GetParameterSlotMap();

protected:
	/// Metadata
	ShaderStubMetadata*				Metadata;

	string							Source;

	ShaderSource2*					ShaderSrc;

	/// Maps stub parameters to stub slots
	map<ShaderStubParameter*, Slot*>	ParameterSlotMap;
};


