#include <include/dom/node.h>
#include <include/base/helpers.h>
#include <algorithm>

Slot::Slot(NodeType type, Node* owner, SharedString name, bool isMultiSlot,
           bool AutoAddToSlotList)
           : mOwner(owner)
           , mType(type)
           , mIsMultiSlot(isMultiSlot) {
  ASSERT(mOwner != nullptr);
  mNode = NULL;
  mName = name;
  if (AutoAddToSlotList) {
    mOwner->mSlots.push_back(this);
    mOwner->ReceiveMessage(this, NodeMessage::SLOT_STRUCTURE_CHANGED, nullptr);
  }
}


Slot::~Slot() {
  DisconnectAll(false);
}


bool Slot::Connect(Node* target) {
  if (target && NodeType::ALLOW_ALL != mType && target->GetType() != mType) {
    /// TODO: use ASSERT only
    ERR("Slot and operator type mismatch");
    ASSERT(false);
    return false;
  }
  if (!mIsMultiSlot) {
    if (mNode != target) {
      if (mNode) mNode->DisconnectFromSlot(this);
      mNode = target;
      if (mNode) mNode->ConnectToSlot(this);

      mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
    }
  } else {
    ASSERT(target != nullptr);
    for (Node* node : mMultiNodes) {
      if (node == target) {
        ERR("Node already connected to slot.");
        return false;
      }
    }
    mMultiNodes.push_back(target);
    mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
  }
  return true;
}


void Slot::Disconnect(Node* target) {
  if (mIsMultiSlot) {
    for (auto it = mMultiNodes.begin(); it != mMultiNodes.end(); it++) {
      if (*it == target) {
        target->DisconnectFromSlot(this);
        mMultiNodes.erase(it);
        mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
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
  } else {
    if (mNode) mNode->DisconnectFromSlot(this);
    mNode = nullptr;
  }

  if (notifyOwner) {
    mOwner->ReceiveMessage(this, NodeMessage::SLOT_CONNECTION_CHANGED);
  }
}


Node* Slot::GetNode() const {
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
  UINT index = it - mMultiNodes.begin();

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


Node::Node(NodeType type)
  : mType(type) 
{
  mIsDirty = true;
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


void Node::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      CheckConnections();
      //SendMessage(NodeMessage::TRANSITIVE_CONNECTION_CHANGED);
      /// Fall through:
    case NodeMessage::VALUE_CHANGED:
      if (!mIsDirty) {
        mIsDirty = true;
        SendMsg(NodeMessage::VALUE_CHANGED);
      }
      break;
    default:
      break;
  }
}


void Node::SendMsg(NodeMessage message, const void* payload) {
  for (Slot* slot : mDependants) {
    slot->mOwner->ReceiveMessage(slot, message, payload);
  }
}


void Node::ReceiveMessage(Slot* slot, NodeMessage message, const void* payload) {
  HandleMessage(slot, message, payload);
  onMessageReceived(slot, message, payload);
}


void Node::CheckConnections() {
  for(Slot* slot : mSlots) {
    /// TODO: handle multislots
    if (!slot->mIsMultiSlot && slot->GetNode() == NULL) {
      mIsProperlyConnected = false;
      return;
    }
  }
  mIsProperlyConnected = true;
}


void Node::Evaluate() {
  if (mIsDirty && mIsProperlyConnected) {
    for(Slot* slot : mSlots) {
      slot->GetNode()->Evaluate();
    }
    Operate();
    mIsDirty = false;
  }
}


Node::~Node() {
  const vector<Slot*>& deps = GetDependants();
  while (deps.size()) {
    deps.back()->Disconnect(this);
  }
}


Node* Node::Clone() const {
  ASSERT(false);
  return NULL;
}


