#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>


/// Array for variable sizes in bytes
const int gVariableByteSizes[] = {
#undef ITEM
#define ITEM(name, type) sizeof(NodeTypes<NodeType::name>::Type),
  VALUETYPE_LIST
};


Slot::Slot(NodeType type, Node* owner, SharedString name, bool isMultiSlot,
           bool isPublic, bool isSerializable)
           : mOwner(owner)
           , mType(type)
           , mIsMultiSlot(isMultiSlot) {
  ASSERT(mOwner != nullptr);
  mNode = nullptr;
  mName = name;
  mOwner->AddSlot(this, isPublic, isSerializable);
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
    mOwner->ReceiveMessage(NodeMessage::MULTISLOT_CONNECTION_ADDED, this, target);
  } else {
    if (mNode != target) {
      if (mNode) mNode->DisconnectFromSlot(this);
      mNode = target;
      if (mNode) mNode->ConnectToSlot(this);
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
        mOwner->ReceiveMessage(NodeMessage::MULTISLOT_CONNECTION_REMOVED, this, target);
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
      mOwner->ReceiveMessage(NodeMessage::MULTISLOT_CLEARED, this);
    }
  } else {
    if (mNode) mNode->DisconnectFromSlot(this);
    mNode = nullptr;
    if (notifyOwner) {
      mOwner->ReceiveMessage(NodeMessage::SLOT_CONNECTION_CHANGED, this);
    }
  }
}


Node* Slot::GetAbstractNode() const {
  ASSERT(!mIsMultiSlot);
  return mNode;
}


const vector<Node*>& Slot::GetMultiNodes() const {
  ASSERT(mIsMultiSlot);
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


Node::Node(NodeType type)
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
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      CheckConnections();
      //SendMessage(NodeMessage::TRANSITIVE_CONNECTION_CHANGED);
      /// Fall through:
    case NodeMessage::VALUE_CHANGED:
      if (mIsUpToDate) {
        mIsUpToDate = false;
        SendMsg(NodeMessage::VALUE_CHANGED);
      }
      break;
    default:
      break;
  }
}


void Node::SendMsg(NodeMessage message, void* payload) {
  for (Slot* slot : mDependants) {
    slot->mOwner->ReceiveMessage(message, slot, payload);
  }
}


void Node::ReceiveMessage(NodeMessage message, Slot* slot, void* payload) {
  HandleMessage(message, slot, payload);
  onSniffMessage(message, slot, payload);
}


void Node::CheckConnections() {
  for (Slot* slot : mPublicSlots) {
    /// TODO: handle multislots
    if (!slot->mIsMultiSlot && slot->GetAbstractNode() == NULL) {
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
        for (Node* node : slot->GetMultiNodes()) {
          node->Update();
        }
      } else {
        Node* node = slot->GetAbstractNode();
        if (node) node->Update();
      }
    }
    Operate();
    mIsUpToDate = true;
  }
}


Node::~Node() {
  /// Remove all watchers. Create a copy of the event hook, because watchers remove
  /// themselves from the callback list while the Event object iterates through it.
  //auto eventCopy = onSniffMessage;
  //eventCopy(NodeMessage::NODE_REMOVED, nullptr, nullptr); 
    
  /// Remove all watchers.
  while (onSniffMessage.mDelegates.size()) {
    auto callback = *onSniffMessage.mDelegates.begin();
    callback(NodeMessage::NODE_REMOVED, nullptr, nullptr);

    /// Check whether the watcher was removed. 
    ASSERT(onSniffMessage.mDelegates.size() == 0 || 
           *onSniffMessage.mDelegates.begin() != callback);
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
  ReceiveMessage(NodeMessage::NODE_NAME_CHANGED);
}

const string& Node::GetName() const {
  return mName;
}

void Node::SetPosition(const Vec2 position) {
  mPosition.x = floorf(position.x);
  mPosition.y = floorf(position.y);
  ReceiveMessage(NodeMessage::NODE_POSITION_CHANGED);
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

void Node::AddSlot(Slot* slot, bool isPublic, bool isSerializable) {
  /// All public slots need to be serializable
  ASSERT(!isPublic || isSerializable);

  if (isPublic) {
    ASSERT(find(mPublicSlots.begin(), mPublicSlots.end(), slot) == mPublicSlots.end());
    mPublicSlots.push_back(slot);
  }
  if (isSerializable) {
    mSerializableSlotsByName[slot->GetName()] = slot;
  }
}

void Node::ClearSlots() {
  mPublicSlots.clear();
  mSerializableSlotsByName.clear();
}

const vector<Slot*>& Node::GetPublicSlots() {
  return mPublicSlots;
}

const unordered_map<SharedString, Slot*>& Node::GetSerializableSlots() {
  return mSerializableSlotsByName;
}

