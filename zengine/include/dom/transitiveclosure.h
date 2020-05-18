#pragma once

#include <functional>

#include "node.h"

class TransitiveClosure {
public:
  TransitiveClosure(const std::shared_ptr<Node>& root, bool useVirtualNodes,
    std::function<bool(Slot*)> includeSlotPredicate = nullptr);

  std::vector<std::shared_ptr<Node>>& GetTopologicalOrder();
  void Invalidate();

private:
  void Traverse(const std::shared_ptr<Node>& node);

  std::shared_ptr<Node> mRoot;
  std::function<bool(Slot*)> mIncludeSlotPredicate;
  bool mUseVirtualNodes;

  std::vector<std::shared_ptr<Node>> mTopologicalOrder;
  std::set<std::shared_ptr<Node>> mVisited;
};

//void Node::GenerateTransitiveClosure(std::vector<std::shared_ptr<Node>>& oResult,
//  bool includeHiddenSlots) {
//  TransitiveClosure tmp(this->shared_from_this(), includeHiddenSlots, oResult);
//}
