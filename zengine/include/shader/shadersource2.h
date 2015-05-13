#pragma once

#include "../dom/node.h"

struct ShaderSourceVariable
{
	ShaderSourceVariable(NodeType Type, const string& Name);

	NodeType		Type;
	string			Name;
	/// TODO: layout, vertexformat usage etc.
};

struct ShaderSourceUniform
{
	NodeType		Type;
	string			Name;
	Slot*			TheSlot;
};


/// All metadata collected from a shader source.
struct ShaderSourceMetadata 
{
	ShaderSourceMetadata(
		const vector<OWNERSHIP ShaderSourceVariable*>& Inputs,
		const vector<OWNERSHIP ShaderSourceVariable*>& Outputs,
		const vector<OWNERSHIP ShaderSourceUniform*>& Uniforms);
	~ShaderSourceMetadata();

	const vector<ShaderSourceVariable*>	Inputs;
	const vector<ShaderSourceVariable*>	Outputs;
	const vector<ShaderSourceUniform*>	Uniforms;
};


class ShaderSource2 : public Node
{
public:
	ShaderSource2();
	virtual ~ShaderSource2();

	virtual Node*				Clone() const override;

	Slot						Stub;

protected:
	virtual void				HandleMessage(Slot* S, NodeMessage Message,
									const void* Payload) override;
	void						CollectMetadata();
	void						GenerateSource();

	ShaderSourceMetadata*		Metadata;
	string						Source;
	
	static const string&		GetTypeString(NodeType Type);
};

