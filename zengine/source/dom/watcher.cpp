#include <include/dom/watcher.h>
#include <include/dom/node.h>

Watcher::Watcher(const shared_ptr<Node>& node)
  : mNode(node)
{}

Watcher::~Watcher() {
  ASSERT(mNode == nullptr);
}

shared_ptr<Node> Watcher::GetNode() {
  return mNode;
}

void Watcher::ChangeNode(Node* node) {
  mNode = node;
  OnRedraw();
}

void Watcher::OnDeleteNode() {}
void Watcher::OnRedraw() {}
void Watcher::OnNameChange() {}
void Watcher::OnSlotGhostChange(Slot* slot) {}
void Watcher::OnSlotConnectionChanged(Slot* slot) {}
void Watcher::OnSlotStructureChanged() {}
void Watcher::OnGraphPositionChanged() {}
void Watcher::OnSplineControlPointsChanged() {}
void Watcher::OnTimeEdited(float time) {}
void Watcher::OnChildNameChange() {}
