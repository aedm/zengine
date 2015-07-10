#include "stubanalyzer.h"
#include <include/shaders/valuestubslot.h>
#include <include/shaders/stubnode.h>
#include <include/shaders/shadernode.h>
#include <include/nodes/valuenodes.h>

REGISTER_NODECLASS(StubNode, "Stub");

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
  , mShader(nullptr) {
}

StubNode::StubNode(const StubNode& original)
  : Node(original)
  , mMetadata(nullptr)
  , mShader(nullptr) {
  SetStubSource(original.GetStubSource());
}

StubNode::~StubNode() {
  SafeDelete(mMetadata);
}

void StubNode::SetStubSource(const string& source) {
  mSource = source;

  for (Slot* slot : GetPublicSlots()) delete slot;
  ClearSlots();

  mParameterSlotMap.clear();
  mParameterNameSlotMap.clear();

  SafeDelete(mMetadata);
  mMetadata = StubAnalyzer::FromText(mSource.c_str());

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

ShaderNode* StubNode::GetShaderSource() {
  if (mShader == nullptr) {
    mShader = new ShaderNode();
    mShader->mStub.Connect(this);
  }
  return mShader;
}

ShaderStubMetadata* StubNode::GetStubMetadata() const {
  return mMetadata;
}

Slot* StubNode::GetSlotByParameter(ShaderStubParameter* parameter) {
  return mParameterSlotMap.at(parameter);
}

Slot* StubNode::GetSlotByParameterName(const string& name) {
  return mParameterNameSlotMap.at(name);
}

Node* StubNode::Clone() const {
  return new StubNode(*this);
}

const string& StubNode::GetStubSource() const {
  return mSource;
}

void StubNode::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
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

ShaderStubMetadata::ShaderStubMetadata(const string& _name, NodeType _returnType,
    const string& _strippedSource, 
    OWNERSHIP const vector<ShaderStubParameter*>& _parameters,
    const vector<ShaderStubGlobal*>& _globals,
    const vector<ShaderStubVariable*>& _inputs,
    const vector<ShaderStubVariable*>& _outputs)
  : name(_name)
  , returnType(_returnType)
  , parameters(_parameters)
  , globals(_globals)
  , strippedSource(_strippedSource)
  , inputs(_inputs)
  , outputs(_outputs) {}

ShaderStubMetadata::~ShaderStubMetadata() {
  for (auto x : parameters) delete(x);
}
