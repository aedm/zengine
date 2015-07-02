#include "stubanalyzer.h"
#include <include/shaders/valuestubslot.h>
#include <include/shaders/shaderstub.h>
#include <include/shaders/shadersource2.h>
#include <include/nodes/valuenodes.h>

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

ShaderStub::ShaderStub(const string& source)
  : Node(NodeType::SHADER_STUB)
  , mMetadata(nullptr)
  , mShaderSrc(nullptr) {
  SetStubSource(source);
}

ShaderStub::ShaderStub(const ShaderStub& original)
  : Node(original)
  , mMetadata(nullptr)
  , mShaderSrc(nullptr) {
  SetStubSource(original.GetStubSource());
}

ShaderStub::~ShaderStub() {
  SafeDelete(mMetadata);
}

void ShaderStub::SetStubSource(const string& source) {
  mSource = source;

  /// TODO: dont do this
  for (Slot* slot : mSlots) delete slot;
  mSlots.clear();
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

ShaderSource2* ShaderStub::GetShaderSource() {
  if (mShaderSrc == nullptr) {
    mShaderSrc = new ShaderSource2();
    mShaderSrc->mStub.Connect(this);
  }
  return mShaderSrc;
}

ShaderStubMetadata* ShaderStub::GetStubMetadata() const {
  return mMetadata;
}

Slot* ShaderStub::GetSlotByParameter(ShaderStubParameter* parameter) {
  return mParameterSlotMap.at(parameter);
}

Slot* ShaderStub::GetSlotByParameterName(const string& name) {
  return mParameterNameSlotMap.at(name);
}

Node* ShaderStub::Clone() const {
  return new ShaderStub(*this);
}

const string& ShaderStub::GetStubSource() const {
  return mSource;
}

void ShaderStub::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
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
