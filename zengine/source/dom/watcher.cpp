#include <include/dom/watcher.h>
#include <include/dom/node.h>

Watcher::Watcher(Node* node)
  : mNode(node)
{
  mNode->mWatchers.insert(this);
}

Watcher::~Watcher() {
  if (mNode) mNode->mWatchers.erase(this);
}


void Watcher::ChangeNode(Node* node) {
  if (mNode) mNode->mWatchers.erase(this);
  mNode = node;
  if (mNode) mNode->mWatchers.insert(this);
  OnRedraw();
}


void Watcher::OnRedraw() {}
void Watcher::OnNameChange() {}
void Watcher::OnSlotConnectionChanged(Slot* slot) {}
void Watcher::OnSlotStructureChanged() {}
void Watcher::OnGraphPositionChanged() {}
void Watcher::OnSplineControlPointsChanged() {}
void Watcher::OnSplineTimeChanged() {}
