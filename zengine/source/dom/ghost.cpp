#include <include/dom/ghost.h>
#include <include/dom/nodetype.h>

/// TODO: use plain TransitiveClosure for this
class GhostTransitiveClosure {
public:
  GhostTransitiveClosure(const std::shared_ptr<Node>& root, 
    std::vector<std::shared_ptr<Node>>& oResult)
  {
    mResult = &oResult;
    Traverse(root);
  }

private:
  /// Returns true if a ghost slots was found in the subtree
  bool Traverse(const std::shared_ptr<Node>& node) {
    if (mVisited.find(node) != mVisited.end()) {
      const bool hadGhostSlot = mHadGhostSlot.find(node) != mHadGhostSlot.end();
      return hadGhostSlot;
    }
    mVisited.insert(node);

    const std::vector<Slot*>& slots = node->GetSlots();
    bool foundGhostSlot = false;

    for (Slot* slot : slots) {
      if (slot->IsGhost()) {
        foundGhostSlot = true;
        continue;
      }
      if (!slot->mIsPublic) {
        continue;
      }

      if (slot->mIsMultiSlot) {
        for (const std::shared_ptr<Node>& refNodes : slot->GetDirectMultiNodes()) {
          foundGhostSlot |= Traverse(refNodes);
        }
      }
      else if (!slot->IsDefaulted()) {
        std::shared_ptr<Node> dependency = slot->GetDirectNode();
        if (dependency != nullptr) {
          foundGhostSlot |= Traverse(dependency);
        }
      }
    }

    if (foundGhostSlot) {
      mResult->push_back(node);
      mHadGhostSlot.insert(node);
    }
    return foundGhostSlot;
  }

  std::vector<std::shared_ptr<Node>>* mResult;
  std::set<std::shared_ptr<Node>> mVisited;
  std::set<std::shared_ptr<Node>> mHadGhostSlot;
};

Ghost::Ghost()
  : mOriginalNode(this, "Original", false, false, true, true)
  , mMainInternalNode(this, std::string(), false, false, false, false)
{
  Regenerate();
}

bool Ghost::IsGhostNode() {
  return true;
}

bool Ghost::IsDirectReference() const
{
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

std::shared_ptr<Node> Ghost::GetReferencedNode() {
  return mMainInternalNode.GetReferencedNode();
}

void Ghost::Regenerate() {
  const std::shared_ptr<Node> root = mOriginalNode.GetReferencedNode();
  std::vector<std::shared_ptr<Node>> topologicalOrder;
  if (root != nullptr) {
    GhostTransitiveClosure(root, topologicalOrder);
  }

  std::set<std::shared_ptr<Node>> newInternalNodes;
  std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> newNodeMapping;
  ClearSlots();
  AddSlot(&mOriginalNode);

  if (topologicalOrder.empty()) {
    /// No ghost slots, just reference the original node
    mMainInternalNode.Connect(mOriginalNode.GetReferencedNode());
  }
  else {
    for (std::shared_ptr<Node>& node : topologicalOrder) {
      std::shared_ptr<Node> internalNode;
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
      const auto& originalSlots = node->GetSlots();
      const auto& internalNodeSlots = internalNode->GetSlots();
      const size_t slotCount = originalSlots.size();
      ASSERT(slotCount == internalNode->GetSlots().size());
      for (UINT i = 0; i < slotCount; i++) {
        Slot* originalSlot = originalSlots[i];
        /// TODO: review if this check is necessary
        if (!originalSlot->mIsPublic) continue;
        Slot* internalSlot = internalNodeSlots[i];
        ASSERT(originalSlot->mName == internalSlot->mName);
        if (originalSlot->IsGhost()) {
          /// TODO: initialize value
          AddSlot(internalSlot);
          if (originalSlot->IsDefaulted() &&
            internalSlot->GetDirectNode() == originalSlot->GetDirectNode()) {
            internalSlot->DisconnectAll(true);
          }
        }
        else {
          if (originalSlot->mIsMultiSlot) {
            for (const auto& connectedNode : originalSlot->GetDirectMultiNodes()) {
              auto it2 = newNodeMapping.find(connectedNode);
              std::shared_ptr<Node> nodeToConnect = (it2 == newNodeMapping.end())
                ? connectedNode : it2->second;
              internalSlot->Connect(nodeToConnect);
            }
          }
          else {
            std::shared_ptr<Node> connectedNode = originalSlot->GetDirectNode();
            auto it2 = newNodeMapping.find(connectedNode);
            std::shared_ptr<Node> nodeToConnect = (it2 == newNodeMapping.end())
              ? connectedNode : it2->second;
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


