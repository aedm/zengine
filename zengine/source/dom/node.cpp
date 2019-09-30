#include <include/dom/node.h>
#include <include/dom/ghost.h>
#include <include/dom/watcher.h>
#include <include/base/helpers.h>
#include <algorithm>
#include <utility>

MessageQueue TheMessageQueue;

void MessageQueue::Enqueue(const shared_ptr<Node>& source, const shared_ptr<Node>& target,
  MessageType type, Slot* slot) {
  const Message message = { source, target, slot, type };
  if (mMessageSet.find(message) != mMessageSet.end()) return;
  mMessageQueue.push_back(message);
  mMessageSet.insert(message);

  if (!mIsInProgress) {
    mIsInProgress = true;
    ProcessAllMessages();
    mIsInProgress = false;
  }
}

void MessageQueue::ProcessAllMessages() {
  while (!mMessageQueue.empty()) {
    Message message = mMessageQueue.front();
    mMessageQueue.pop_front();
    mMessageSet.erase(message);
    message.mTarget->ReceiveMessage(&message);
  }
}

void MessageQueue::RemoveNode(Node* node) {
start:
  for (auto it = mMessageQueue.begin(); it != mMessageQueue.end(); ++it) {
    Message message = *it;
    if (message.mTarget == nullptr || message.mTarget.get() != node) continue;
    mMessageQueue.erase(it);
    mMessageSet.erase(message);
    goto start;  // NOLINT
  }
}

bool Message::operator<(const Message& other) const {
  if (this->mTarget < other.mTarget) return true;
  if (this->mTarget == other.mTarget) {
    if (this->mSource < other.mSource) return true;
    if (this->mSource == other.mSource) {
      if (this->mSlot < other.mSlot) return true;
      if (this->mSlot == other.mSlot) {
        if (this->mType < other.mType) return true;
      }
    }
  }
  return false;
}

Slot::Slot(Node* owner, string name, bool isMultiSlot, bool isPublic, 
  bool isSerializable, bool isTraversable)
  : mName(std::move(name))
  , mIsMultiSlot(isMultiSlot) 
  , mOwner(owner)
{
  ASSERT(owner != nullptr);
  owner->AddSlot(this, isPublic, isSerializable, isTraversable);
}


Slot::~Slot() {
  DisconnectAll(false);
}


std::shared_ptr<Node> Slot::GetOwner() const
{
  return mOwner->shared_from_this();
}

bool Slot::IsOwnerExpired() const
{
  return mOwner->weak_from_this().expired();
}

bool Slot::Connect(const shared_ptr<Node>& target) {
  if (target && !DoesAcceptNode(target)) {
    ERR("Slot doesn't accept node.");
    DEBUGBREAK;
    return false;
  }
  if (mIsMultiSlot) {
    ASSERT(target != nullptr);
    for (shared_ptr<Node>& node : mMultiNodes) {
      if (node == target) {
        // TODO: this needs to be fixed
        //DEBUGBREAK("Node already connected to slot.");
        return false;
      }
    }
    mMultiNodes.push_back(target);
    target->ConnectToSlot(this);
    mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this, target);
  }
  else {
    if (mNode != target) {
      if (mNode) mNode->DisconnectFromSlot(this);
      mNode = target;
      if (mNode) mNode->ConnectToSlot(this);
      mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this, target);
    }
  }
  return true;
}


void Slot::Disconnect(const shared_ptr<Node>& target) {
  if (mIsMultiSlot) {
    for (auto it = mMultiNodes.begin(); it != mMultiNodes.end(); ++it) {
      if (*it == target) {
        target->DisconnectFromSlot(this);
        mMultiNodes.erase(it);
        mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this, target);
        return;
      }
    }
    ERR("Node was not found.");
  }
  else {
    ASSERT(target == mNode);
    DisconnectAll(true);
  }
}


void Slot::DisconnectAll(bool notifyOwner) {
  if (mIsMultiSlot) {
    for (auto& mMultiNode : mMultiNodes) {
      mMultiNode->DisconnectFromSlot(this);
    }
    mMultiNodes.clear();
    if (notifyOwner) {
      mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this);
    }
  }
  else {
    if (mNode) mNode->DisconnectFromSlot(this);
    mNode = nullptr;
    if (notifyOwner) {
      mOwner->EnqueueMessage(MessageType::SLOT_CONNECTION_CHANGED, this);
    }
  }
}


shared_ptr<Node> Slot::GetReferencedNode() const {
  ASSERT(!mIsMultiSlot);
  return mNode ? mNode->GetReferencedNode() : nullptr;
}


shared_ptr<Node> Slot::GetDirectNode() const {
  ASSERT(!mIsMultiSlot);
  return mNode;
}


UINT Slot::GetMultiNodeCount() const {
  return UINT(mMultiNodes.size());
}


shared_ptr<Node> Slot::GetReferencedMultiNode(UINT index) const {
  ASSERT(index >= 0 && index < mMultiNodes.size());
  return mMultiNodes[index]->GetReferencedNode();
}


const vector<shared_ptr<Node>>& Slot::GetDirectMultiNodes() const {
  return mMultiNodes;
}


void Slot::ChangeNodeIndex(const shared_ptr<Node>& node, UINT targetIndex) {
  ASSERT(mIsMultiSlot);
  ASSERT(targetIndex < mMultiNodes.size());

  const auto it = std::find(mMultiNodes.begin(), mMultiNodes.end(), node);
  ASSERT(it != mMultiNodes.end());
  const UINT index = UINT(it - mMultiNodes.begin());

  const shared_ptr<Node> tempNode = mMultiNodes[index];
  if (index < targetIndex) {
    for (UINT i = index; i < targetIndex; i++) {
      mMultiNodes[i] = mMultiNodes[i + 1];
    }
    mMultiNodes[targetIndex] = tempNode;
  }
  else {
    for (UINT i = index; i > targetIndex; i--) {
      mMultiNodes[i] = mMultiNodes[i - 1];
    }
    mMultiNodes[targetIndex] = tempNode;
  }
}

const shared_ptr<Node>& Slot::operator[](UINT index) {
  ASSERT(mIsMultiSlot);
  ASSERT(index < mMultiNodes.size());
  return mMultiNodes[index];
}

bool Slot::DoesAcceptNode(const shared_ptr<Node>& node) const {
  return true;
}

bool Slot::IsDefaulted() {
  /// Plain slots don't have default values, only ValueSlots
  return false;
}

void Slot::SetGhost(bool isGhost) {
  mGhostSlot = isGhost;
  mOwner->EnqueueMessage(MessageType::SLOT_GHOST_FLAG_CHANGED, this);
}

bool Slot::IsGhost() const
{
  return mGhostSlot;
}

Node::Node(bool isForwarderNode) {
  mIsUpToDate = false;
  mIsProperlyConnected = true;
}


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


void Node::HandleMessage(Message* message) {}


shared_ptr<Node> Node::GetReferencedNode() {
  return this->shared_from_this();
}


bool Node::IsGhostNode() {
  return false;
}
  
void Node::Dispose() {
  RemoveAllWatchers();
  while (!mDependants.empty()) {
    mDependants.at(0)->Disconnect(this->shared_from_this());
  }
}

void Node::CopyFrom(const shared_ptr<Node>& node) {}

void Node::SendMsg(MessageType message) {
  for (Slot* slot : mDependants) {
    if (!slot->IsOwnerExpired()) {
      slot->GetOwner()->EnqueueMessage(message, slot, this->shared_from_this());
    }
  }
}


void Node::ReceiveMessage(Message* message) {
  switch (message->mType) {
  case MessageType::SLOT_CONNECTION_CHANGED:
    CheckConnections();
    /// Notifies dependants about transitive closure change
    EnqueueMessage(MessageType::TRANSITIVE_CLOSURE_CHANGED, message->mSlot,
      message->mSource);
    NotifyWatchers(&Watcher::OnSlotConnectionChanged, message->mSlot);
    break;
  case MessageType::TRANSITIVE_CLOSURE_CHANGED:
    /// Bubbles up transitive closure notifications
    SendMsg(MessageType::TRANSITIVE_CLOSURE_CHANGED);
    break;
  case MessageType::NEEDS_REDRAW:
    /// Bubbles up redraw command
    SendMsg(MessageType::NEEDS_REDRAW);
    NotifyWatchers(&Watcher::OnRedraw);
    break;
  case MessageType::NODE_NAME_CHANGED:
    if (message->mSource) {
      NotifyWatchers(&Watcher::OnChildNameChange);
    }
    break;
  case MessageType::SLOT_GHOST_FLAG_CHANGED:
    EnqueueMessage(MessageType::TRANSITIVE_GHOST_CHANGED, message->mSlot,
      message->mSource);
    NotifyWatchers(&Watcher::OnSlotGhostChange, message->mSlot);
    break;
  case MessageType::TRANSITIVE_GHOST_CHANGED:
    SendMsg(MessageType::TRANSITIVE_GHOST_CHANGED);
    break;
  default:
    break;
  }

  HandleMessage(message);
}


void Node::CheckConnections() {
  for (Slot* slot : mPublicSlots) {
    /// TODO: handle multislots
    if (!slot->mIsMultiSlot && slot->GetReferencedNode() == nullptr) {
      mIsProperlyConnected = false;
      return;
    }
  }
  mIsProperlyConnected = true;
}


void Node::EnqueueMessage(MessageType message, Slot* slot /*= nullptr*/,
  const shared_ptr<Node>& sender /*= nullptr*/)
{
  /// Glory to C++17
  if (this->weak_from_this().expired()) return;
  TheMessageQueue.Enqueue(sender, this->shared_from_this(), message, slot);
}

void Node::Update() {
  if (!mIsUpToDate && mIsProperlyConnected) {
    for (Slot* slot : mPublicSlots) {
      if (slot->mIsMultiSlot) {
        for (UINT i = 0; i < slot->GetMultiNodeCount(); i++) {
          slot->GetReferencedMultiNode(i)->Update();
        }
      }
      else {
        shared_ptr<Node> node = slot->GetReferencedNode();
        if (node) node->Update();
      }
    }
    Operate();
    mIsUpToDate = true;
  }
}


Node::~Node() {
  /// Don't send any more messages to this node.
  TheMessageQueue.RemoveNode(this);
  RemoveAllWatchers();

  const vector<Slot*>& deps = GetDependants();
  while (!deps.empty()) {
    Slot* slot = deps.back();
    shared_ptr<Node> owner = slot->GetOwner();

    /// If a ghost is made of this node, delete it.
    if (owner->IsGhostNode()) {
      const shared_ptr<Ghost> ghost = PointerCast<Ghost>(owner);
      if (ghost->mOriginalNode.GetDirectNode().get() == this) {
        //delete ghost;
        NOT_IMPLEMENTED;
        continue;
      }
    }
    slot->Disconnect(this->shared_from_this());
  }
}

void Node::SetName(const string& name) {
  mName = name;
  EnqueueMessage(MessageType::NODE_NAME_CHANGED);
  NotifyWatchers(&Watcher::OnNameChange);
}

const string& Node::GetName() const {
  return mName;
}

void Node::SetPosition(const Vec2 position) {
  mPosition.x = floorf(position.x);
  mPosition.y = floorf(position.y);
  NotifyWatchers(&Watcher::OnGraphPositionChanged);
}

Vec2 Node::GetPosition() const {
  return mPosition;
}

void Node::SetSize(const Vec2 size) {
  mSize = size;
  EnqueueMessage(MessageType::NODE_NAME_CHANGED);
  SendMsg(MessageType::NODE_NAME_CHANGED);
}

Vec2 Node::GetSize() const {
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
    mSerializableSlotsByName[slot->mName] = slot;
  }
}

void Node::ClearSlots() {
  mPublicSlots.clear();
  mSerializableSlotsByName.clear();
  mTraversableSlots.clear();
}

const vector<Slot*>& Node::GetPublicSlots() const
{
  return mPublicSlots;
}

const std::vector<Slot*>& Node::GetTraversableSlots() const
{
  return mTraversableSlots;
}

const unordered_map<string, Slot*>& Node::GetSerializableSlots() const
{
  return mSerializableSlotsByName;
}

void Node::RemoveWatcher(const shared_ptr<Watcher>& watcher) {
  const auto it = std::find(mWatchers.begin(), mWatchers.end(), watcher);
  if (it == mWatchers.end()) return;
  ASSERT(watcher->mNode.get() == this);
  watcher->OnRemovedFromNode();
  mWatchers.erase(it);
}

void Node::RemoveAllWatchers()
{
  while (!mWatchers.empty()) {
    RemoveWatcher(*mWatchers.begin());
  }
}

void Node::AssignWatcher(const shared_ptr<Watcher>& watcher) {
  mWatchers.insert(watcher);
  watcher->ChangeNode(this->shared_from_this());
}

class TransitiveClosure {
public:
  TransitiveClosure(const shared_ptr<Node>& root, bool includeHiddenSlots,
    vector<shared_ptr<Node>>& oResult) {
    mResult = &oResult;
    mIncludeHiddenSlots = includeHiddenSlots;
    Traverse(root);
  }

private:
  void Traverse(const shared_ptr<Node>& node) {
    if (mVisited.find(node) != mVisited.end()) return;
    mVisited.insert(node);

    const vector<Slot*>& slots =
      mIncludeHiddenSlots ? node->GetTraversableSlots() : node->GetPublicSlots();

    for (Slot* slot : slots) {
      if (slot->mIsMultiSlot) {
        for (const shared_ptr<Node>& directNode : slot->GetDirectMultiNodes()) {
          Traverse(directNode);
        }
      }
      else if (!slot->IsDefaulted()) {
        shared_ptr<Node> dependency = slot->GetDirectNode();
        if (dependency != nullptr) Traverse(dependency);
      }
    }

    mResult->push_back(node);
  }

  bool mIncludeHiddenSlots;
  vector<shared_ptr<Node>>* mResult;
  set<shared_ptr<Node>> mVisited;
};

void Node::GenerateTransitiveClosure(vector<shared_ptr<Node>>& oResult,
  bool includeHiddenSlots) {
  TransitiveClosure tmp(this->shared_from_this(), includeHiddenSlots, oResult);
}
