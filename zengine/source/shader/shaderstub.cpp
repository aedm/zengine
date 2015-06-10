#include "stubanalyzer.h"
#include <include/shader/shaderstub.h>
#include <include/shader/shadersource2.h>

ShaderStub::ShaderStub(const string& _Source)
	: Node(NodeType::SHADER_STUB, "ShaderStub")
	, mMetadata(nullptr)
	, mShaderSrc(nullptr)
{
	SetStubSource(_Source);
}

ShaderStub::ShaderStub(const ShaderStub& Original)
	: Node(Original)
	, mMetadata(nullptr)
	, mShaderSrc(nullptr)
{
	SetStubSource(Original.GetStubSource());
}

ShaderStub::~ShaderStub()
{
	SafeDelete(mMetadata);
}

void ShaderStub::SetStubSource(const string& _Source)
{
	mSource = _Source;

	/// TODO: dont do this
	for (Slot* slot : mSlots) delete slot;
	mSlots.clear();
	mParameterSlotMap.clear();
	
	SafeDelete(mMetadata);
	mMetadata = StubAnalyzer::FromText(mSource.c_str());

	if (mMetadata == nullptr) return;

	for (auto param : mMetadata->parameters)
	{
		Slot* slot = new Slot(param->type, this, make_shared<string>(param->name), false);
		mParameterSlotMap[param] = slot;
	}
}

ShaderSource2* ShaderStub::GetShaderSource()
{
	if (mShaderSrc == nullptr)
	{
		mShaderSrc = new ShaderSource2();
		mShaderSrc->mStub.Connect(this);
	}
	return mShaderSrc;
}

ShaderStubMetadata* ShaderStub::GetStubMetadata() const
{
	return mMetadata;
}

const map<ShaderStubParameter*, Slot*>& ShaderStub::GetParameterSlotMap()
{
	return mParameterSlotMap;
}

Node* ShaderStub::Clone() const
{
	return new ShaderStub(*this);
}

const string& ShaderStub::GetStubSource() const
{
	return mSource;
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
	: name(_Name)
	, returnType(_ReturnType)
	, parameters(_Parameters)
	, globals(_Globals)
	, strippedSource(_StrippedSource)
	, inputs(_Inputs)
	, outputs(_Outputs)
{}

ShaderStubMetadata::~ShaderStubMetadata()
{
	for (auto x : parameters) delete(x);
}
