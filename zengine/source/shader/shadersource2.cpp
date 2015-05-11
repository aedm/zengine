#include <include/shader/shadersource2.h>
#include <include/shader/shaderstub.h>

ShaderSource2::ShaderSource2()
	: Node(NodeType::SHADER_SOURCE, "ShaderSource")
	, Stub(NodeType::SHADER_STUB, this, nullptr)
{

}

ShaderSource2::~ShaderSource2()
{}

void ShaderSource2::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
	case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
		CollectMetadata();
		break;
		//case NodeMessage::VALUE_CHANGED:
		//	break;
		//case NodeMessage::NEEDS_REDRAW:
		//	break;
	default:
		break;
	}
}

void ShaderSource2::CollectMetadata()
{
	SafeDelete(Metadata);
	ShaderStub* stub = static_cast<ShaderStub*>(Stub.GetNode());
	if (stub == nullptr) return;

	ShaderStubMetadata* stubMeta = stub->GetStubMetadata();

	NOT_IMPLEMENTED;
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

