#pragma once

#include "shadersource2.h"
#include "../dom/node.h"
#include "../shaders/shaders.h"
#include <map>

using namespace std;

struct PassUniform
{
	UniformId			Handle;
	Node*				TheNode;
	ShaderGlobalType	GlobalType;
	NodeType			Type;
};

/// A renderpass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
class Pass : public Node
{
public:
	Pass();
	virtual ~Pass();

	virtual Node*		Clone() const override;

	Slot				FragmentStub;
	Slot				VertexStub;

	/// Hidden, automatic slots
	Slot				FragmentSource;
	Slot				VertexSource;

	void				Set(Globals* Global);

	/// Returns true if pass can be used
	bool				IsComplete();

	const vector<ShaderAttributeDesc>&		GetUsedAttributes();
	
protected:
	virtual void		HandleMessage(Slot* S, NodeMessage Message, const void* Payload) override;

	void				BuildRenderPipeline();
	
	ShaderHandle		Handle;
	vector<PassUniform>	Uniforms;
	vector<ShaderAttributeDesc> Attributes;
};