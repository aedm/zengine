#include <include/shader/shadersource2.h>
#include "shadersourcebuilder.h"
#include <include/shader/shaderstub.h>

ShaderSource2::ShaderSource2()
	: Node(NodeType::SHADER_SOURCE, "ShaderSource")
	, mStub(NodeType::SHADER_STUB, this, make_shared<string>("Stub"))
	, metadata(nullptr)
{}

ShaderSource2::~ShaderSource2()
{
	SafeDelete(metadata);
	/// There are dynamically created slots that won't be automatically 
	/// disconencted.
	for (Slot* slot : mSlots)
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
		if (S == &mStub)
		{
			ShaderSourceBuilder::FromStub(static_cast<ShaderStub*>(mStub.GetNode()), this);
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
	return mSource;
}

const ShaderSourceMetadata* ShaderSource2::GetMetadata() const
{
	return metadata;
}


ShaderSourceMetadata::ShaderSourceMetadata(
	const vector<OWNERSHIP ShaderSourceVariable*>& _Inputs,
	const vector<OWNERSHIP ShaderSourceVariable*>& _Outputs,
	const vector<OWNERSHIP ShaderSourceUniform*>& _Uniforms)
	: inputs(_Inputs)
	, outputs(_Outputs)
	, uniforms(_Uniforms)
{}

ShaderSourceMetadata::~ShaderSourceMetadata()
{
	for (auto x : inputs) delete x;
	for (auto x : outputs) delete x;
	for (auto x : uniforms) delete x;
}


ShaderSourceVariable::ShaderSourceVariable(NodeType _Type, const string& _Name)
	: name(_Name)
	, type(_Type)
{}

ShaderSourceUniform::ShaderSourceUniform(NodeType _Type, const string& _Name, Node* Nd,
	ShaderGlobalType _GlobalType)
	: type(_Type)
	, name(_Name)
	, node(Nd)
	, globalType(_GlobalType)
{}
