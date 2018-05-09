#include <include/dom/ghost.h>
#include <include/dom/nodetype.h>

class GhostTransitiveClosure {
public:
  GhostTransitiveClosure(const shared_ptr<Node>& root, vector<shared_ptr<Node>>& oResult) {
    mResult = &oResult;
    Traverse(root);
  }

private:
  /// Returns true if a ghost slots was found in the subtree
  bool Traverse(const shared_ptr<Node>& node) {
    if (mVisited.find(node) != mVisited.end()) return false;
    mVisited.insert(node);

    const vector<Slot*>& slots = node->GetPublicSlots();
    bool foundGhostSlot = false;

    for (Slot* slot : slots) {
      if (slot->IsGhost()) {
        foundGhostSlot = true;
        continue;
      }

      if (slot->mIsMultiSlot) {
        for (const shared_ptr<Node>& node : slot->GetDirectMultiNodes()) {
          foundGhostSlot |= Traverse(node);
        }
      }
      else if (!slot->IsDefaulted()) {
        shared_ptr<Node> dependency = slot->GetDirectNode();
        if (dependency != nullptr) {
          foundGhostSlot |= Traverse(dependency);
        }
      }
    }

    if (foundGhostSlot) mResult->push_back(node);
    return foundGhostSlot;
  }

  vector<shared_ptr<Node>>* mResult;
  set<shared_ptr<Node>> mVisited;
};

static SharedString OriginalSlotName = make_shared<string>("Original");

Ghost::Ghost()
  : Node()
  , mOriginalNode(this, OriginalSlotName, false, false, true, true)
  , mMainInternalNode(this, nullptr, false, false, false, false)
{
  Regenerate();
}

bool Ghost::IsGhostNode() {
  return true;
}

bool Ghost::IsDirectReference() {
  return mMainInternalNode.GetDirectNode() == mOriginalNode.GetDirectNode();
}

void Ghost::HandleMessage(Message* message)
{
  switch (message->mType) {
  case MessageType::TRANSITIVE_GHOST_CHANGED:
  case MessageType::TRANSITIVE_CLOSURE_CHANGED:
    if (message->mSource == mOriginalNode.GetDirectNode()) {
      Regenerate();
    }
    break;
  case MessageType::SLOT_CONNECTION_CHANGED:
    if (message->mSlot == &mOriginalNode) {
      Regenerate();
    }
  default:
    break;
  }
}

shared_ptr<Node> Ghost::GetReferencedNode() {
  return mMainInternalNode.GetReferencedNode();
}

void Ghost::Regenerate() {
  shared_ptr<Node> root = mOriginalNode.GetReferencedNode();
  vector<shared_ptr<Node>> topologicalOrder;
  if (root != nullptr) GhostTransitiveClosure(root, topologicalOrder);

  set<shared_ptr<Node>> newInternalNodes;
  map<shared_ptr<Node>, shared_ptr<Node>> newNodeMapping;
  ClearSlots();
  AddSlot(&mOriginalNode, false, true, true);

  if (topologicalOrder.size() == 0) {
    /// No ghost slots, just reference the original node
    mMainInternalNode.Connect(mOriginalNode.GetReferencedNode());
  }
  else {
    for (shared_ptr<Node>& node : topologicalOrder) {
      shared_ptr<Node> internalNode;
      auto it = mNodeMapping.find(node);
      if (it == mNodeMapping.end()) {
        /// Node was not copied before
        internalNode = NodeRegistry::GetInstance()->GetNodeClass(node)->Manufacture();
      }
      else {
        internalNode = it->second;
      }
      internalNode->CopyFrom(node);
      newInternalNodes.insert(internalNode);
      newNodeMapping[node] = internalNode;

      /// Connect slots
      const auto& originalSlots = node->GetPublicSlots();
      const auto& internalNodeSlots = internalNode->GetPublicSlots();
      size_t slotCount = originalSlots.size();
      ASSERT(slotCount == internalNode->GetPublicSlots().size());
      for (UINT i = 0; i < slotCount; i++) {
        Slot* originalSlot = originalSlots[i];
        Slot* internalSlot = internalNodeSlots[i];
        ASSERT(*originalSlot->GetName() == *internalSlot->GetName());
        if (originalSlot->IsGhost()) {
          /// TODO: initialize value
          AddSlot(internalSlot, true, true, true);
          if (originalSlot->IsDefaulted() &&
            internalSlot->GetDirectNode() == originalSlot->GetDirectNode()) {
            internalSlot->DisconnectAll(true);
          }
        }
        else {
          if (originalSlot->mIsMultiSlot) {
            for (const auto& connectedNode : originalSlot->GetDirectMultiNodes()) {
              auto it = newNodeMapping.find(connectedNode);
              shared_ptr<Node> nodeToConnect = (it == newNodeMapping.end())
                ? connectedNode : it->second;
              internalSlot->Connect(nodeToConnect);
            }
          }
          else {
            shared_ptr<Node> connectedNode = originalSlot->GetDirectNode();
            auto it = newNodeMapping.find(connectedNode);
            shared_ptr<Node> nodeToConnect = (it == newNodeMapping.end())
              ? connectedNode : it->second;
            internalSlot->Connect(nodeToConnect);
          }
        }
      }
    }
    mMainInternalNode.Connect(newNodeMapping.at(root));
  }

  mInternalNodes = newInternalNodes;
  mNodeMapping = newNodeMapping;

  /// TODO: detect when there was no actual change
  NotifyWatchers(&Watcher::OnRedraw);
  NotifyWatchers(&Watcher::OnSlotStructureChanged);
  SendMsg(MessageType::TRANSITIVE_GHOST_CHANGED);
}


