#include <include/dom/node.h>
#include <include/dom/watcher.h>
#include <include/base/helpers.h>
#include <algorithm>

/// Array for variable sizes in bytes
const int gVariableByteSizes[] = {
#undef ITEM
#define ITEM(name, type) sizeof(NodeTypes<NodeType::name>::Type),
  VALUETYPE_LIST
};


Slot::Slot(NodeType type, Node* owner, SharedString name, bool isMultiSlot,
           bool isPublic, bool isSerializable, bool isTraversable)
           : mOwner(owner)
           , mType(type)
           , mIsMultiSlot(isMultiSlot) {
  ASSERT(mOwner != nullptr);
  mNode = nullptr;
  mName = name;
  mOwner->AddSlot(this, isPublic, isSerializable, isTraversable);
  if (isPublic) {
    mOwner->ReceiveMessage(NodeMessage::SLOT_STRUCTURE_CHANGED, this, nullptr);
  }
}


Slot::~Slot() {
  DisconnectAll(false);
}


bool Slot::Connect(Node* target) {
  if (target && NodeType::ALLOW_ALL != mType && target->GetType() != mType) {
    DEBUGBREAK("Slot and operator type mismatch");
    return false;
  }
  if (mIsMultiSlot) {
    ASSERT(target != nullptr);
    for (Node* node : mMultiNodes) {
      if (node == target) {
        DEBUGBREAK("Node already connected to slot.");
        return false;
      }
    }
    mMultiNodes.push_back(target);
    target->ConnectToSlot(this);
    /// TODO: merge these
    mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
  } else {
    if (mNode != target) {
      if (mNode) mNode->DisconnectFromSlot(this);
      mNode = target;
      if (mNode) mNode->ConnectToSlot(this);
      /// TODO: merge these
      mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
    }
  }
  return true;
}


void Slot::Disconnect(Node* target) {
  if (mIsMultiSlot) {
    for (auto it = mMultiNodes.begin(); it != mMultiNodes.end(); it++) {
      if (*it == target) {
        target->DisconnectFromSlot(this);
        mMultiNodes.erase(it);
        mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
        return;
      }
    }
    ERR("Node was not found.");
  } else {
    ASSERT(target == mNode);
    ASSERT(target != nullptr);
    DisconnectAll(true);
  }
}


void Slot::DisconnectAll(bool notifyOwner) {
  if (mIsMultiSlot) {
    for (auto it = mMultiNodes.begin(); it != mMultiNodes.end(); it++) {
      (*it)->DisconnectFromSlot(this);
    }
    mMultiNodes.clear();
    if (notifyOwner) {
      mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED);
    }
  } else {
    if (mNode) mNode->DisconnectFromSlot(this);
    mNode = nullptr;
    if (notifyOwner) {
      mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
    }
  }
}


Node* Slot::GetReferencedNode() const {
  ASSERT(!mIsMultiSlot);
  return mNode ? mNode->GetReferencedNode() : nullptr;
}


Node* Slot::GetDirectNode() const {
  ASSERT(!mIsMultiSlot);
  return mNode;
}


UINT Slot::GetMultiNodeCount() const {
  return UINT(mMultiNodes.size());
}


Node* Slot::GetReferencedMultiNode(UINT index) const {
  ASSERT(index >= 0 && index < mMultiNodes.size());
  return mMultiNodes[index]->GetReferencedNode();
}


std::vector<Node*> Slot::GetDirectMultiNodes() const {
  return mMultiNodes;
}


SharedString Slot::GetName() {
  return mName;
}


void Slot::ChangeNodeIndex(Node* node, UINT targetIndex) {
  ASSERT(mIsMultiSlot);
  ASSERT(targetIndex < mMultiNodes.size());

  auto it = std::find(mMultiNodes.begin(), mMultiNodes.end(), node);
  ASSERT(it != mMultiNodes.end());
  UINT index = UINT(it - mMultiNodes.begin());

  Node* tempNode = mMultiNodes[index];
  if (index < targetIndex) {
    for (UINT i = index; i < targetIndex; i++) {
      mMultiNodes[i] = mMultiNodes[i + 1];
    }
    mMultiNodes[targetIndex] = tempNode;
  } else {
    for (UINT i = index; i > targetIndex; i--) {
      mMultiNodes[i] = mMultiNodes[i - 1];
    }
    mMultiNodes[targetIndex] = tempNode;
  }
}

Node* Slot::operator[](UINT index) {
  ASSERT(mIsMultiSlot);
  ASSERT(index < mMultiNodes.size());
  return mMultiNodes[index];
}

bool Slot::DoesAcceptType(NodeType type) const {
  return mType == NodeType::ALLOW_ALL || type == mType;
}

bool Slot::IsDefaulted() {
  /// Plain slots don't have default values, only ValueSlots
  return false;
}


Node::Node(NodeType type, bool isForwarderNode)
  : mType(type)
{
  mIsUpToDate = false;
  mIsProperlyConnected = true;
}


Node::Node(const Node& original)
  : mType(original.mType)
{}


void Node::ConnectToSlot(Slot* slot) {
  /// No need for typecheck, the Slot already did that.
  mDependants.push_back(slot);
}


void Node::DisconnectFromSlot(Slot* slot) {
  mDependants.erase(std::remove(mDependants.begin(), mDependants.end(), slot), 
                    mDependants.end());
}


const vector<Slot*>& Node::GetDependants() const {
  return mDependants;
}


NodeType Node::GetType() const {
  return mType;
}


void Node::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  // Overload this
}


Node* Node::GetReferencedNode() {
  return this;
}

void Node::SendMsg(NodeMessage message, void* payload) {
  for (Slot* slot : mDependants) {
    slot->mOwner->ReceiveMessage(message, slot, payload);
  }
}


void Node::ReceiveMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      CheckConnections();
      /// Notifies dependants about transitive closure change
      ReceiveMessage(NodeMessage::TRANSITIVE_CLOSURE_CHANGED);
      NotifyWatchers(&Watcher::OnSlotConnectionChanged, slot);
      break;
    case NodeMessage::TRANSITIVE_CLOSURE_CHANGED:
      /// Bubbles up transitive closure notifications
      SendMsg(NodeMessage::TRANSITIVE_CLOSURE_CHANGED);
      break;
    case NodeMessage::NEEDS_REDRAW:
      /// Bubbles up redraw command
      SendMsg(NodeMessage::NEEDS_REDRAW);
      NotifyWatchers(&Watcher::OnRedraw);
      break;
    //case NodeMessage::VALUE_CHANGED:
    //  /// TODO: figure out a proper way to propagate value changes
    //  if (mIsUpToDate) {
    //    mIsUpToDate = false;
    //    SendMsg(NodeMessage::VALUE_CHANGED);
    //  }
    //  break;
    case NodeMessage::NODE_NAME_CHANGED:
      NotifyWatchers(&Watcher::OnChildNameChange);
      break;
    default:
      break;
  }

  HandleMessage(message, slot, payload);
}


void Node::CheckConnections() {
  for (Slot* slot : mPublicSlots) {
    /// TODO: handle multislots
    if (!slot->mIsMultiSlot && slot->GetReferencedNode() == NULL) {
      mIsProperlyConnected = false;
      return;
    }
  }
  mIsProperlyConnected = true;
}


void Node::Update() {
  if (!mIsUpToDate && mIsProperlyConnected) {
    for(Slot* slot : mPublicSlots) {
      if (slot->mIsMultiSlot) {
        for (UINT i = 0; i < slot->GetMultiNodeCount(); i++) {
          slot->GetReferencedMultiNode(i)->Update();
        }
      } else {
        Node* node = slot->GetReferencedNode();
        if (node) node->Update();
      }
    }
    Operate();
    mIsUpToDate = true;
  }
}


Node::~Node() {
  while (mWatchers.size()) {
    auto it = mWatchers.begin();
    (*it)->mNode = nullptr;
    (*it)->OnDeleteNode();
    mWatchers.erase(it);
  }

  const vector<Slot*>& deps = GetDependants();
  while (deps.size()) {
    Slot* slot = deps.back();
    slot->Disconnect(this);
    //deps.back()->Disconnect(this);
  }
}

void Node::SetName(const string& name) {
  mName = name;
  NotifyWatchers(&Watcher::OnNameChange);
  SendMsg(NodeMessage::NODE_NAME_CHANGED);
}

const string& Node::GetName() const {
  return mName;
}

void Node::SetPosition(const Vec2 position) {
  mPosition.x = floorf(position.x);
  mPosition.y = floorf(position.y);
  NotifyWatchers(&Watcher::OnGraphPositionChanged);
}

const Vec2 Node::GetPosition() const {
  return mPosition;
}

void Node::SetSize(const Vec2 size) {
  mSize = size;
  ReceiveMessage(NodeMessage::NODE_POSITION_CHANGED);
}

const Vec2 Node::GetSize() const {
  return mSize;
}

void Node::AddSlot(Slot* slot, bool isPublic, bool isSerializable, bool isTraversable) {
  /// All public slots need to be serializable
  ASSERT(!isPublic || isSerializable);

  if (isPublic) {
    ASSERT(find(mPublicSlots.begin(), mPublicSlots.end(), slot) == mPublicSlots.end());
    mPublicSlots.push_back(slot);
  }
  if (isTraversable) {
    ASSERT(find(mTraversableSlots.begin(), mTraversableSlots.end(), slot) == mTraversableSlots.end());
    mTraversableSlots.push_back(slot);
  }
  if (isSerializable) {
    mSerializableSlotsByName[slot->GetName()] = slot;
  }
}

void Node::ClearSlots() {
  mPublicSlots.clear();
  mSerializableSlotsByName.clear();
  mTraversableSlots.clear();
}

const vector<Slot*>& Node::GetPublicSlots() {
  return mPublicSlots;
}

const std::vector<Slot*>& Node::GetTraversableSlots() {
  return mTraversableSlots;
}

const unordered_map<SharedString, Slot*>& Node::GetSerializableSlots() {
  return mSerializableSlotsByName;
}

void Node::RemoveWatcher(Watcher* watcher) {
  for (auto it = mWatchers.begin(); it != mWatchers.end(); it++) {
    if (it->get() == watcher) {
      ASSERT(watcher->mNode == this);
      watcher->mNode = nullptr;
      mWatchers.erase(it);
      return;
    }
  }
}

void Node::AssignWatcher(shared_ptr<Watcher> watcher) {
  mWatchers.insert(watcher);
  watcher->ChangeNode(this);
}

class TransitiveClosure {
public:
  TransitiveClosure(Node* root, bool includeHiddenSlots, vector<Node*>& oResult) {
    mResult = &oResult;
    mIncludeHiddenSlots = includeHiddenSlots;
    Traverse(root);
  }

private:
  void Traverse(Node* node) {
    if (mVisited.find(node) != mVisited.end()) return;
    mVisited.insert(node);

    const vector<Slot*>& slots = 
      mIncludeHiddenSlots ? node->GetTraversableSlots() : node->GetPublicSlots();

    for (Slot* slot : slots) {
      if (slot->mIsMultiSlot) {
        for (UINT i = 0; i < slot->GetMultiNodeCount(); i++) {
          Traverse(slot->GetReferencedMultiNode(i));
        }
      } else if (!slot->IsDefaulted()) {
        Node* dependency = slot->GetReferencedNode();
        if (dependency != nullptr) Traverse(dependency);
      }
    }

    mResult->push_back(node);
  }

  bool mIncludeHiddenSlots;
  vector<Node*>* mResult;
  set<Node*> mVisited;
};

void Node::GenerateTransitiveClosure(vector<Node*>& oResult, bool includeHiddenSlots) {
  TransitiveClosure tmp(this, includeHiddenSlots, oResult);
}
