#include "stubanalyzer.h"
#include <include/shaders/valuestubslot.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadernode.h>

REGISTER_NODECLASS(StubNode, "Stub");

static SharedString SourceSlotName = make_shared<string>("Source");

const EnumMapperA GlobalUniformMapper[] = {
#undef ITEM
#define ITEM(name, type, token) { "g" MAGIC(token), (UINT)ShaderGlobalType::name },
  GLOBALUSAGE_LIST
  {"", -1}
};

/// Array for global uniform types
const NodeType GlobalUniformTypes[] = {
#undef ITEM
#define ITEM(name, type, token) NodeType::type,
  GLOBALUSAGE_LIST
};

const int GlobalUniformOffsets[] = {
#undef ITEM
#define ITEM(name, type, token) offsetof(Globals, token),
  GLOBALUSAGE_LIST
};

StubNode::StubNode()
  : Node(NodeType::SHADER_STUB)
  , mMetadata(nullptr)
  , mShader(nullptr)
  , mSource(this, SourceSlotName)
{}

StubNode::StubNode(const StubNode& original)
  : Node(original)
  , mMetadata(nullptr)
  , mShader(nullptr) 
  , mSource(this, SourceSlotName)
{
  SetStubSource(original.mSource.Get());
}

StubNode::~StubNode() {
  ResetStubSlots();
  SafeDelete(mMetadata);
}

void StubNode::SetStubSource(const string& source) {
  ResetStubSlots();

  ASSERT(mSource.IsDefaulted());
  mSource.SetDefaultValue(source);

  SafeDelete(mMetadata);
  mMetadata = StubAnalyzer::FromText(mSource.Get().c_str());

  if (mMetadata == nullptr) return;

  for (auto param : mMetadata->parameters) {
    Slot* slot = nullptr;
    switch (param->type) {
      case NodeType::FLOAT:
        slot = new ValueStubSlot<NodeType::FLOAT>(this, make_shared<string>(param->name));
        break;
      case NodeType::VEC2:
        slot = new ValueStubSlot<NodeType::VEC2>(this, make_shared<string>(param->name));
        break;
      case NodeType::VEC3:
        slot = new ValueStubSlot<NodeType::VEC3>(this, make_shared<string>(param->name));
        break;
      case NodeType::VEC4:
        slot = new ValueStubSlot<NodeType::VEC4>(this, make_shared<string>(param->name));
        break;
      case NodeType::TEXTURE:
        slot = new TextureSlot(this, make_shared<string>(param->name));
        break;
      default:
        SHOULDNT_HAPPEN;
        break;
    }
    mParameterSlotMap[param] = slot;
    mParameterNameSlotMap[param->name] = slot;
  }
}

ShaderNode* StubNode::GetShader() {
  if (mShader == nullptr) {
    mShader = new ShaderNode();
    mShader->mStub.Connect(this);
  }
  return mShader;
}

StubMetadata* StubNode::GetStubMetadata() const {
  return mMetadata;
}

Slot* StubNode::GetSlotByParameter(StubParameter* parameter) {
  return mParameterSlotMap.at(parameter);
}

Slot* StubNode::GetSlotByParameterName(const string& name) {
  return mParameterNameSlotMap.at(name);
}

void StubNode::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      CheckConnections();
      /// Fall through:
    case NodeMessage::TRANSITIVE_CONNECTION_CHANGED:
      SendMsg(NodeMessage::TRANSITIVE_CONNECTION_CHANGED);
      break;
    default:
      break;
  }
}

void StubNode::ResetStubSlots() {
  for (auto slotPair : mParameterSlotMap) delete(slotPair.second);
  ClearSlots();
  AddSlot(&mSource, true, true);
}

StubMetadata::StubMetadata(const string& _name, NodeType _returnType,
    const string& _strippedSource, 
    OWNERSHIP const vector<StubParameter*>& _parameters,
    const vector<StubGlobal*>& _globals,
    const vector<StubVariable*>& _inputs,
    const vector<StubVariable*>& _outputs)
  : name(_name)
  , returnType(_returnType)
  , parameters(_parameters)
  , globals(_globals)
  , strippedSource(_strippedSource)
  , inputs(_inputs)
  , outputs(_outputs) {}

StubMetadata::~StubMetadata() {
  for (auto x : parameters) delete(x);
}
