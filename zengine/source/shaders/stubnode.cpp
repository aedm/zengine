#include "stubanalyzer.h"
#include <include/shaders/valuestubslot.h>
#include <include/shaders/stubnode.h>
#include <include/nodes/texturenode.h>

REGISTER_NODECLASS(StubNode, "Stub");

static SharedString SourceSlotName = make_shared<string>("Source");

const EnumMapperA GlobalUniformMapper[] = {
#undef ITEM
#define ITEM(name, type) { "g" MAGIC(name), (UINT)GlobalUniformUsage::name },
  GLOBALUNIFORM_LIST
  {"", -1}
};

const EnumMapperA GlobalSamplerMapper[] = {
#undef ITEM
#define ITEM(name) { "g" MAGIC(name), (UINT)GlobalSamplerUsage::name },
  GLOBALSAMPLER_LIST
  {"", -1}
};

/// Array for global uniform types
const ShaderValueType GlobalUniformTypes[] = {
#undef ITEM
#define ITEM(name, type) type,
  GLOBALUNIFORM_LIST
};

const int GlobalUniformOffsets[] = {
#undef ITEM
#define ITEM(name, type) offsetof(Globals, name),
  GLOBALUNIFORM_LIST
};

const int GlobalSamplerOffsets[] = {
#undef ITEM
#define ITEM(name) offsetof(Globals, name),
  GLOBALSAMPLER_LIST
};

StubNode::StubNode()
  : mMetadata(nullptr)
  , mSource(this, SourceSlotName, false, false)
{}


StubNode::~StubNode() {
  mSource.GetNode()->RemoveAllWatchers();
  for (auto slotPair : mParameterSlotMap) delete(slotPair.second);
  mParameterSlotMap.clear();
  mParameterNameSlotMap.clear();
  ClearSlots();
  SafeDelete(mMetadata);
}

void StubNode::Operate() {
  /// Regenerate metadata
  StubMetadata* metadata = StubAnalyzer::FromText(mSource.Get().c_str());
  if (metadata == nullptr) {
    /// Keep old shader. Not strictly correct, but better than deleting everything 
    /// with a typo.
    return;
  }
  SafeDelete(mMetadata);
  mMetadata = metadata;

  /// Clear previous list of public slots (but not the Slot objects)
  ClearSlots();
  mParameterSlotMap.clear();

  /// Create a new list of slots
  for (auto param : mMetadata->parameters) {
    auto it = mParameterNameSlotMap.find(*param->mName);
    bool canReuseSlot = false;
    if (it != mParameterNameSlotMap.end()) {
      switch (param->mType)
      {
      case StubParameter::Type::FLOAT:
        canReuseSlot = IsInsanceOf<ValueStubSlot<float>*>(it->second);
        break;
      case StubParameter::Type::VEC2:
        canReuseSlot = IsInsanceOf<ValueStubSlot<Vec2>*>(it->second);
        break;
      case StubParameter::Type::VEC3:
        canReuseSlot = IsInsanceOf<ValueStubSlot<Vec3>*>(it->second);
        break;
      case StubParameter::Type::VEC4:
        canReuseSlot = IsInsanceOf<ValueStubSlot<Vec4>*>(it->second);
        break;
      case StubParameter::Type::MATRIX44:
        canReuseSlot = IsInsanceOf<ValueStubSlot<Matrix>*>(it->second);
        break;
      case StubParameter::Type::SAMPLER2D:
        canReuseSlot = IsInsanceOf<TextureSlot*>(it->second);
        break;
      default:
        break;
      }
    }
    if (canReuseSlot) {
      /// This slot was used before, reuse it.
      /// "isTraversable" is false since it's already in the mTraversableSlots vector.
      AddSlot(it->second, true, true, true);
      mParameterSlotMap[param] = it->second;
      it->second = nullptr;
    }
    else {
      /// Generate new slot.
      Slot* slot = nullptr;
      switch (param->mType) {
      case StubParameter::Type::FLOAT:
        slot = new ValueStubSlot<float>(this, param->mName);
        break;
      case StubParameter::Type::VEC2:
        slot = new ValueStubSlot<Vec2>(this, param->mName);
        break;
      case StubParameter::Type::VEC3:
        slot = new ValueStubSlot<Vec3>(this, param->mName);
        break;
      case StubParameter::Type::VEC4:
        slot = new ValueStubSlot<Vec4>(this, param->mName);
        break;
      case StubParameter::Type::SAMPLER2D:
        slot = new TextureSlot(this, param->mName);
        break;
      default:
        SHOULD_NOT_HAPPEN;
        break;
      }
      mParameterSlotMap[param] = slot;
    }
  }

  /// Delete unused slots
  for (auto it = mParameterNameSlotMap.begin(); it != mParameterNameSlotMap.end(); ++it) {
    SafeDelete(it->second);
  }
  mParameterNameSlotMap.clear();

  /// Index the new slots
  for (auto it = mParameterSlotMap.begin(); it != mParameterSlotMap.end(); ++it) {
    ASSERT(it->second != nullptr);
    mParameterNameSlotMap[*it->first->mName] = it->second;
  }

  NotifyWatchers(&Watcher::OnSlotStructureChanged);
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

void StubNode::CopyFrom(const shared_ptr<Node>& node)
{
  shared_ptr<StubNode> original = PointerCast<StubNode>(node);
  mSource.SetDefaultValue(original->mSource.Get());
  mIsUpToDate = false;
  if (original->mMetadata) {
    SetName(original->mMetadata->name);
  }
  Update();
}

void StubNode::HandleMessage(Message* message) {
  /// Stubs send a VALUE_CHANGED message if the shader needs to be rebuilt
  switch (message->mType) {
  case MessageType::VALUE_CHANGED:
    if (message->mSlot == &mSource) {
      mIsUpToDate = false;
      /// TODO: make sure slots are updated outside the Update() mechanism
      Update();
      SendMsg(MessageType::VALUE_CHANGED);
    }
    else if (IsPointerOf<StubNode>(message->mSlot->GetReferencedNode())) {
      SendMsg(MessageType::VALUE_CHANGED);
    }
    else {
      SendMsg(MessageType::NEEDS_REDRAW);
    }
    break;
  case MessageType::SLOT_CONNECTION_CHANGED:
    if (message->mSlot == &mSource) {
      mIsUpToDate = false;
      /// TODO: make sure slots are updated outside the Update() mechanism
      Update();
    }
    SendMsg(MessageType::VALUE_CHANGED);
    break;
  case MessageType::NODE_NAME_CHANGED:
    // TODO: implement and use GetDefaultNode for value slots
    if (message->mSource == nullptr) {
      mSource.GetNode()->SetName(GetName());
    }
    break;
  default:
    break;
  }
}

StubMetadata::StubMetadata(const string& _name, StubParameter::Type _returnType,
  const string& _strippedSource,
  OWNERSHIP const vector<StubParameter*>& _parameters,
  const vector<StubGlobalUniform*>& _globalUniforms,
  const vector<StubGlobalSampler*>& _globalSamplers,
  const vector<StubInOutVariable*>& _inputs,
  const vector<StubInOutVariable*>& _outputs)
  : name(_name)
  , returnType(_returnType)
  , parameters(_parameters)
  , globalUniforms(_globalUniforms)
  , globalSamplers(_globalSamplers)
  , strippedSource(_strippedSource)
  , inputs(_inputs)
  , outputs(_outputs) {}

StubMetadata::~StubMetadata() {
  for (auto x : parameters) delete(x);
}

bool StubParameter::IsValidShaderValueType(Type type) {
  switch (type) {
  case StubParameter::Type::FLOAT:
  case StubParameter::Type::VEC2:
  case StubParameter::Type::VEC3:
  case StubParameter::Type::VEC4:
  case StubParameter::Type::MATRIX44:
    return true;
  default:
    return false;
  }
}

ShaderValueType StubParameter::ToShaderValueType(Type type)
{
  switch (type) {
  case StubParameter::Type::FLOAT:
    return ShaderValueType::FLOAT;
  case StubParameter::Type::VEC2:
    return ShaderValueType::VEC2;
  case StubParameter::Type::VEC3:
    return ShaderValueType::VEC3;
  case StubParameter::Type::VEC4:
    return ShaderValueType::VEC4;
  case StubParameter::Type::MATRIX44:
    return ShaderValueType::MATRIX44;
  default:
    SHOULD_NOT_HAPPEN;
    return ShaderValueType(-1);
  }
}

ShaderValueType NodeToValueType(const shared_ptr<Node>& node) {
  if (IsPointerOf<FloatNode>(node)) return ShaderValueType::FLOAT;
  if (IsPointerOf<Vec2Node>(node)) return ShaderValueType::VEC2;
  if (IsPointerOf<Vec3Node>(node)) return ShaderValueType::VEC3;
  if (IsPointerOf<Vec4Node>(node)) return ShaderValueType::VEC4;
  if (IsPointerOf<MatrixNode>(node)) return ShaderValueType::MATRIX44;
  SHOULD_NOT_HAPPEN;
  return ShaderValueType(-1);
}