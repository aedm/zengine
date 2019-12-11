#include "stubanalyzer.h"
#include <include/shaders/valuestubslot.h>
#include <include/shaders/stubnode.h>
#include <include/nodes/texturenode.h>
#include <include/nodes/buffernode.h>
#include <utility>

REGISTER_NODECLASS(StubNode, "Stub");

const EnumMapA<GlobalUniformUsage> GlobalUniformMapper = {
#undef ITEM
#define ITEM(name, type) { "g" MAGIC(name), GlobalUniformUsage::name },
  GLOBAL_UNIFORM_LIST
};

const EnumMapA<GlobalSamplerUsage> GlobalSamplerMapper = {
#undef ITEM
#define ITEM(name) { "g" MAGIC(name), GlobalSamplerUsage::name },
  GLOBAL_SAMPLER_LIST
};

/// Array for global uniform types
const ValueType GlobalUniformTypes[] = {
#undef ITEM
#define ITEM(name, type) type,
  GLOBAL_UNIFORM_LIST
};

const int GlobalUniformOffsets[] = {
#undef ITEM
#define ITEM(name, type) offsetof(Globals, name),
  GLOBAL_UNIFORM_LIST
};

const int GlobalSamplerOffsets[] = {
#undef ITEM
#define ITEM(name) offsetof(Globals, name),
  GLOBAL_SAMPLER_LIST
};

StubNode::StubNode()
  : mSource(this, "Source", false, false)
  , mMetadata(nullptr)
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
  for (auto& param : mMetadata->mParameters) {
    auto it = mParameterNameSlotMap.find(param->mName);
    bool canReuseSlot = false;
    if (it != mParameterNameSlotMap.end()) {
      switch (param->mType)
      {
      case StubParameter::Type::FLOAT:
        canReuseSlot = IsInstanceOf<ValueStubSlot<float>*>(it->second);
        break;
      case StubParameter::Type::VEC2:
        canReuseSlot = IsInstanceOf<ValueStubSlot<vec2>*>(it->second);
        break;
      case StubParameter::Type::VEC3:
        canReuseSlot = IsInstanceOf<ValueStubSlot<vec3>*>(it->second);
        break;
      case StubParameter::Type::VEC4:
        canReuseSlot = IsInstanceOf<ValueStubSlot<vec4>*>(it->second);
        break;
      case StubParameter::Type::MATRIX44:
        canReuseSlot = IsInstanceOf<ValueStubSlot<mat4>*>(it->second);
        break;
      case StubParameter::Type::SAMPLER2D:
      case StubParameter::Type::IMAGE2D:
        canReuseSlot = IsInstanceOf<TextureSlot*>(it->second);
        break;
      case StubParameter::Type::BUFFER:
        canReuseSlot = IsInstanceOf<BufferSlot*>(it->second);
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
        slot = new ValueStubSlot<vec2>(this, param->mName);
        break;
      case StubParameter::Type::VEC3:
        slot = new ValueStubSlot<vec3>(this, param->mName);
        break;
      case StubParameter::Type::VEC4:
        slot = new ValueStubSlot<vec4>(this, param->mName);
        break;
      case StubParameter::Type::SAMPLER2D:
      case StubParameter::Type::IMAGE2D:
        slot = new TextureSlot(this, param->mName);
        break;
      case StubParameter::Type::BUFFER:
        slot = new BufferSlot(this, param->mName);
        break;
      default:
        SHOULD_NOT_HAPPEN;
        break;
      }
      mParameterSlotMap[param] = slot;
    }
  }

  /// Delete unused slots
  for (auto& it : mParameterNameSlotMap)
  {
    SafeDelete(it.second);
  }
  mParameterNameSlotMap.clear();

  /// Index the new slots
  for (auto& it : mParameterSlotMap)
  {
    ASSERT(it.second != nullptr);
    mParameterNameSlotMap[it.first->mName] = it.second;
  }

  NotifyWatchers(&Watcher::OnSlotStructureChanged);
}

StubMetadata* StubNode::GetStubMetadata() const {
  return mMetadata;
}

Slot* StubNode::GetSlotByParameter(StubParameter* parameter) {
  return mParameterSlotMap.at(parameter);
}

Slot* StubNode::GetSlotByParameterName(const std::string& name) {
  return mParameterNameSlotMap.at(name);
}

void StubNode::CopyFrom(const std::shared_ptr<Node>& node)
{
  const std::shared_ptr<StubNode> original = PointerCast<StubNode>(node);
  mSource.SetDefaultValue(original->mSource.Get());
  mIsUpToDate = false;
  if (original->mMetadata) {
    SetName(original->mMetadata->mName);
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

StubMetadata::StubMetadata(std::string name, StubParameter::Type returnType,
                           std::string strippedSource,
                           OWNERSHIP std::vector<StubParameter*> parameters,
                           std::vector<StubGlobalUniform*> globalUniforms,
                           std::vector<StubGlobalSampler*> globalSamplers,
                           std::vector<StubInOutVariable*> inputs,
                           std::vector<StubInOutVariable*> outputs)
  : mName(std::move(name))
  , mReturnType(returnType)
  , mStrippedSource(std::move(strippedSource))
  , mParameters(std::move(parameters))
  , mGlobalUniforms(std::move(globalUniforms))
  , mGlobalSamplers(std::move(globalSamplers))
  , mInputs(std::move(inputs))
  , mOutputs(std::move(outputs))
{}

StubMetadata::~StubMetadata() {
  for (auto x : mParameters) delete(x);
}

bool StubParameter::IsValidValueType(Type type) {
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

ValueType StubParameter::ToValueType(Type type)
{
  switch (type) {
  case StubParameter::Type::FLOAT:
    return ValueType::FLOAT;
  case StubParameter::Type::VEC2:
    return ValueType::VEC2;
  case StubParameter::Type::VEC3:
    return ValueType::VEC3;
  case StubParameter::Type::VEC4:
    return ValueType::VEC4;
  case StubParameter::Type::MATRIX44:
    return ValueType::MATRIX44;
  default:
    SHOULD_NOT_HAPPEN;
    return ValueType(-1);
  }
}

ValueType NodeToValueType(const std::shared_ptr<Node>& node) {
  if (IsPointerOf<ValueNode<float>>(node)) return ValueType::FLOAT;
  if (IsPointerOf<ValueNode<vec2>>(node)) return ValueType::VEC2;
  if (IsPointerOf<ValueNode<vec3>>(node)) return ValueType::VEC3;
  if (IsPointerOf<ValueNode<vec4>>(node)) return ValueType::VEC4;
  if (IsPointerOf<ValueNode<mat4>>(node)) return ValueType::MATRIX44;
  SHOULD_NOT_HAPPEN;
  return ValueType(-1);
}