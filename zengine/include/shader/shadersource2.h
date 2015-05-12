#pragma once

#include "../dom/node.h"

struct ShaderSourceVariable
{
	NodeType		Tpye;
	string			Name;
	/// TODO: layout, vertexformat usage etc.
};

struct ShaderSourceUniform
{
	NodeType		Tpye;
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

	ShaderSourceMetadata*		Metadata;

	void						CollectMetadata();
};

