#pragma once

#include "shadersource2.h"
#include "../dom/node.h"
#include "../shaders/shaders.h"
#include <map>

using namespace std;

struct PassUniform
{
	UniformId			Handle;
	Slot*				TheSlot;
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

	Slot				FragmentShader;
	Slot				VertexShader;

	void				Set(Globals* Global);
	const vector<ShaderAttributeDesc>&		GetUsedAttributes();
	
protected:
	virtual void		HandleMessage(Slot* S, NodeMessage Message, const void* Payload) override;

	void				BuildRenderPipeline();
	
	ShaderHandle		Handle;
	vector<PassUniform>	Uniforms;
	vector<ShaderAttributeDesc> Attributes;
};