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


Ghost::Ghost(const shared_ptr<Node>& originalNode)
  : Node()
  , mOriginalNode(this, nullptr, false, false, true, true)
{
  mOriginalNode.Connect(originalNode);
  Regenerate();
}

bool Ghost::IsGhostNode() {
  return true;
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
  default:
    break;
  }
}

shared_ptr<Node> Ghost::GetReferencedNode() {
  return mMainInternalNode ? mMainInternalNode : mOriginalNode.GetReferencedNode();
}

void Ghost::Regenerate() {
  shared_ptr<Node> root = mOriginalNode.GetReferencedNode();
  vector<shared_ptr<Node>> topologicalOrder;
  if (root != nullptr) GhostTransitiveClosure(root, topologicalOrder);

  set<shared_ptr<Node>> newInternalNodes;
  map<shared_ptr<Node>, shared_ptr<Node>> newNodeMapping;
  ClearSlots();

  if (topologicalOrder.size() == 0) {
    /// No ghost slots, just reference the original node
    mMainInternalNode.reset();
  }
  else {
    for (shared_ptr<Node>& node : topologicalOrder) {
      shared_ptr<Node> internalNode;
      auto it = mNodeMapping.find(node);
      if (it == mNodeMapping.end()) {
        /// Node was not copied before
        internalNode = shared_ptr<Node>(
          NodeRegistry::GetInstance()->GetNodeClass(node)->Manufacture());
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
      UINT slotCount = originalSlots.size();
      ASSERT(slotCount == internalNode->GetPublicSlots().size());
      for (UINT i = 0; i < slotCount; i++) {
        Slot* originalSlot = originalSlots[i];
        Slot* internalSlot = internalNodeSlots[i];
        if (originalSlot->IsGhost()) {
          /// TODO: initialize value
          AddSlot(internalSlot, true, true, true);
        }
        else {
          if (!originalSlot->IsDefaulted()) {
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
              internalSlot->Connect(originalSlot->GetDirectNode());
            }
          }
        }
      }
    }
    mMainInternalNode = newNodeMapping.at(root);
  }

  mInternalNodes = newInternalNodes;
  mNodeMapping = newNodeMapping;
  NotifyWatchers(&Watcher::OnSlotStructureChanged);
}


