#include "stubanalyzer.h"
#include <include/shaders/valuestubslot.h>
#include <include/shaders/stubnode.h>

REGISTER_NODECLASS(StubNode, "Stub");

static SharedString SourceSlotName = make_shared<string>("Source");

const EnumMapperA GlobalUniformMapper[] = {
#undef ITEM
#define ITEM(name, type) { "g" MAGIC(name), (UINT)ShaderGlobalType::name },
  GLOBALUSAGE_LIST
  {"", -1}
};

/// Array for global uniform types
const ValueType GlobalUniformTypes[] = {
#undef ITEM
#define ITEM(name, type) ValueType::type,
  GLOBALUSAGE_LIST
};

const int GlobalUniformOffsets[] = {
#undef ITEM
#define ITEM(name, type) offsetof(Globals, name),
  GLOBALUSAGE_LIST
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
    if (it != mParameterNameSlotMap.end() &&
      it->second->DoesAcceptNode(StaticValueNodesList[int(param->mType)])) {
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
      case ValueType::FLOAT:
        slot = new ValueStubSlot<ValueType::FLOAT>(this, param->mName);
        break;
      case ValueType::VEC2:
        slot = new ValueStubSlot<ValueType::VEC2>(this, param->mName);
        break;
      case ValueType::VEC3:
        slot = new ValueStubSlot<ValueType::VEC3>(this, param->mName);
        break;
      case ValueType::VEC4:
        slot = new ValueStubSlot<ValueType::VEC4>(this, param->mName);
        break;
      case ValueType::TEXTURE:
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

StubMetadata::StubMetadata(const string& _name, ValueType _returnType,
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
