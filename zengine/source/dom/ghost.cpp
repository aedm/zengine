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

shared_ptr<Node> Ghost::GetReferencedNode() {
  return mMainInternalNode ? mMainInternalNode : mOriginalNode.GetReferencedNode();
}

void Ghost::Regenerate() {
  shared_ptr<Node> root = mOriginalNode.GetReferencedNode();
  vector<shared_ptr<Node>> topologicalOrder;
  GhostTransitiveClosure(root, topologicalOrder);

  set<shared_ptr<Node>> newInternalNodes;
  map<shared_ptr<Node>, shared_ptr<Node>> newNodeMapping;

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
      newInternalNodes.insert(internalNode);
      newNodeMapping[node] = internalNode;
    }
    mMainInternalNode = newNodeMapping.at(root);

  }

  mInternalNodes = newInternalNodes;
  mNodeMapping = newNodeMapping;
}


