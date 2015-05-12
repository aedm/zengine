#include <include/shader/shadersource2.h>
#include <include/shader/shaderstub.h>

ShaderSource2::ShaderSource2()
	: Node(NodeType::SHADER_SOURCE, "ShaderSource")
	, Stub(NodeType::SHADER_STUB, this, make_shared<string>("Stub"))
	, Metadata(nullptr)
{}

ShaderSource2::~ShaderSource2()
{}


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
		if (S == &Stub) CollectMetadata();
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
	const map<ShaderStubParameter*, Slot*>& paramSlotMap = stub->GetParameterSlotMap();
	
	for (auto param : stubMeta->Parameters) 
	{
		Slot* stubSlot = paramSlotMap.at(param);
		Slot* slot = new Slot(stubSlot->GetType(), this, stubSlot->GetName(), false, true);
		slot->Connect(stubSlot->GetNode());
	}
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

