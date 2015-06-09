#include <include/shader/shadersource2.h>
#include "shadersourcebuilder.h"
#include <include/shader/shaderstub.h>

ShaderSource2::ShaderSource2()
	: Node(NodeType::SHADER_SOURCE, "ShaderSource")
	, Stub(NodeType::SHADER_STUB, this, make_shared<string>("Stub"))
	, Metadata(nullptr)
{}

ShaderSource2::~ShaderSource2()
{
	SafeDelete(Metadata);
	/// There are dynamically created slots that won't be automatically 
	/// disconencted.
	for (Slot* slot : Slots)
	{
		slot->DisconnectAll(false);
	}
}


Node* ShaderSource2::Clone() const
{
	/// Shader sources get all their data from the slot, so nothing to do here.
	return new ShaderSource2();
}


void ShaderSource2::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
	case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
		if (S == &Stub)
		{
			ShaderSourceBuilder::FromStub(static_cast<ShaderStub*>(Stub.GetNode()), this);
			SendMessage(NodeMessage::VALUE_CHANGED, nullptr);
		}
		break;
	case NodeMessage::VALUE_CHANGED:
		SendMessage(NodeMessage::NEEDS_REDRAW, nullptr);
		break;
	default:
		break;
	}
}

const string& ShaderSource2::GetSource() const
{
	return Source;
}

const ShaderSourceMetadata* ShaderSource2::GetMetadata() const
{
	return Metadata;
}


ShaderSourceMetadata::ShaderSourceMetadata(
	const vector<OWNERSHIP ShaderSourceVariable*>& _Inputs,
	const vector<OWNERSHIP ShaderSourceVariable*>& _Outputs,
	const vector<OWNERSHIP ShaderSourceUniform*>& _Uniforms)
	: Inputs(_Inputs)
	, Outputs(_Outputs)
	, Uniforms(_Uniforms)
{}

ShaderSourceMetadata::~ShaderSourceMetadata()
{
	for (auto x : Inputs) delete x;
	for (auto x : Outputs) delete x;
	for (auto x : Uniforms) delete x;
}


ShaderSourceVariable::ShaderSourceVariable(NodeType _Type, const string& _Name)
	: Name(_Name)
	, Type(_Type)
{}

ShaderSourceUniform::ShaderSourceUniform(NodeType _Type, const string& _Name, Node* Nd,
	ShaderGlobalType _GlobalType)
	: Type(_Type)
	, Name(_Name)
	, TheNode(Nd)
	, GlobalType(_GlobalType)
{}
