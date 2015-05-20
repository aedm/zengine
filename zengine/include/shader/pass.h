#pragma once

#include "../dom/node.h"
#include <include/shader/shadersource2.h>
#include <map>

using namespace std;

struct PassUniform
{
	UniformId			Handle;
	Slot*				TheSlot;

};

/// A renderpass is a way to render an object. Materials consist of several
/// render passes, eg. an opaque pass, a transparent pass, a shadow pass etc.
class Pass : public Node
{
public:
	Pass();
	virtual ~Pass();

	Slot				FragmentShader;
	Slot				VertexShader;

protected:
	virtual void		HandleMessage(Slot* S, NodeMessage Message, const void* Payload) override;

	void				BuildRenderPipeline();

	map<string, ShaderSourceUniform*>	UniformMap;
	vector<ShaderSourceUniform*>	Uniforms;
};