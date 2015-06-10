#include "stubanalyzer.h"
#include <include/shader/shaderstub.h>
#include <include/shader/shadersource2.h>

ShaderStub::ShaderStub(const string& _Source)
	: Node(NodeType::SHADER_STUB, "ShaderStub")
	, Metadata(nullptr)
	, ShaderSrc(nullptr)
{
	SetStubSource(_Source);
}

ShaderStub::ShaderStub(const ShaderStub& Original)
	: Node(Original)
	, Metadata(nullptr)
	, ShaderSrc(nullptr)
{
	SetStubSource(Original.GetStubSource());
}

ShaderStub::~ShaderStub()
{
	SafeDelete(Metadata);
}

void ShaderStub::SetStubSource(const string& _Source)
{
	Source = _Source;

	/// TODO: dont do this
	for (Slot* slot : mSlots) delete slot;
	mSlots.clear();
	ParameterSlotMap.clear();
	
	SafeDelete(Metadata);
	Metadata = StubAnalyzer::FromText(Source.c_str());

	if (Metadata == nullptr) return;

	for (auto param : Metadata->Parameters)
	{
		Slot* slot = new Slot(param->Type, this, make_shared<string>(param->Name), false);
		ParameterSlotMap[param] = slot;
	}
}

ShaderSource2* ShaderStub::GetShaderSource()
{
	if (ShaderSrc == nullptr)
	{
		ShaderSrc = new ShaderSource2();
		ShaderSrc->Stub.Connect(this);
	}
	return ShaderSrc;
}

ShaderStubMetadata* ShaderStub::GetStubMetadata() const
{
	return Metadata;
}

const map<ShaderStubParameter*, Slot*>& ShaderStub::GetParameterSlotMap()
{
	return ParameterSlotMap;
}

Node* ShaderStub::Clone() const
{
	return new ShaderStub(*this);
}

const string& ShaderStub::GetStubSource() const
{
	return Source;
}

void ShaderStub::HandleMessage(Slot* S, NodeMessage Message, const void* Payload)
{
	switch (Message)
	{
	case NodeMessage::SLOT_CONNECTION_CHANGED:
		CheckConnections();
		/// Fall through:
	case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
		SendMessage(NodeMessage::TRANSITIVE_CONNECTION_CHANGED);
		break;
	default:
		break;
	}
}

ShaderStubMetadata::ShaderStubMetadata(const string& _Name, NodeType _ReturnType,
	const string& _StrippedSource,
	OWNERSHIP const vector<ShaderStubParameter*>& _Parameters,
	const vector<ShaderStubGlobal*>& _Globals,
	const vector<ShaderStubVariable*>& _Inputs,
	const vector<ShaderStubVariable*>& _Outputs)
	: Name(_Name)
	, ReturnType(_ReturnType)
	, Parameters(_Parameters)
	, Globals(_Globals)
	, StrippedSource(_StrippedSource)
	, Inputs(_Inputs)
	, Outputs(_Outputs)
{}

ShaderStubMetadata::~ShaderStubMetadata()
{
	for (auto x : Parameters) delete(x);
}
