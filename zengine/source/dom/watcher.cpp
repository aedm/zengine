#include <include/dom/watcher.h>
#include <include/dom/node.h>

Watcher::Watcher(Node* node)
  : mNode(node)
{}

Watcher::~Watcher() {
  ASSERT(mNode == nullptr);
}

Node* Watcher::GetNode() {
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
