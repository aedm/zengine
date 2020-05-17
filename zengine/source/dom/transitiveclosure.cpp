#include <include/dom/transitiveclosure.h>

#include <utility>

TransitiveClosure::TransitiveClosure(const std::shared_ptr<Node>& root,
  std::function<bool(Slot*)> includeSlotPredicate)
  : mRoot(root)
  , mIncludeSlotPredicate(std::move(includeSlotPredicate))
{}

std::vector<std::shared_ptr<Node>>& TransitiveClosure::GetTopologicalOrder() {
  if (mTopologicalOrder.empty()) {
    Traverse(mRoot);
    mVisited.clear();
  }
  return mTopologicalOrder;
}

void TransitiveClosure::Invalidate() {
  mTopologicalOrder.clear();
}

void TransitiveClosure::Traverse(const std::shared_ptr<Node>& node) {
  if (mVisited.find(node) != mVisited.end()) return;
  mVisited.insert(node);

  //const std::vector<Slot*>& slots =
  //  mIncludeHiddenSlots ? node->GetTraversableSlots() : node->GetPublicSlots();
  const std::vector<Slot*>& slots = node->GetSlots();
  for (Slot* slot : slots) {
    if (mIncludeSlotPredicate == nullptr || mIncludeSlotPredicate(slot)) {
      if (slot->mIsMultiSlot) {
        for (const std::shared_ptr<Node>& directNode : slot->GetDirectMultiNodes()) {
          Traverse(directNode);
        }
      }
      else if (!slot->IsDefaulted()) {
        std::shared_ptr<Node> dependency = slot->GetDirectNode();
        if (dependency != nullptr) Traverse(dependency);
      }
    }
  }
  mTopologicalOrder.push_back(node);
}
