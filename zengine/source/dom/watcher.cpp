#include <include/dom/watcher.h>
#include <include/dom/node.h>

Watcher::Watcher(Node* node)
  : mNode(node)
{}

Watcher::~Watcher() {
  ResetNode();
}

Node* Watcher::GetNode() {
  return mNode;
}

void Watcher::ResetNode() {
  if (!mNode) return;
  mNode->RemoveWatcher(this);
  mNode = nullptr;
}

void Watcher::Unwatch() {
  ResetNode();
}

void Watcher::ChangeNode(Node* node) {
  mNode = node;
  OnRedraw();
}

void Watcher::OnRedraw() {}
void Watcher::OnNameChange() {}
void Watcher::OnSlotConnectionChanged(Slot* slot) {}
void Watcher::OnSlotStructureChanged() {}
void Watcher::OnGraphPositionChanged() {}
void Watcher::OnSplineControlPointsChanged() {}
void Watcher::OnSplineTimeChanged() {}
