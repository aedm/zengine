#include <include/dom/watcher.h>
#include <include/dom/node.h>
#include <utility>

Watcher::Watcher(shared_ptr<Node> node)
  : mNode(std::move(node))
{}

Watcher::~Watcher() {
  ASSERT(mNode == nullptr);
}

shared_ptr<Node> Watcher::GetNode() const
{
  return mNode->GetReferencedNode();
}

std::shared_ptr<Node> Watcher::GetDirectNode() const
{
  return mNode;
}

void Watcher::ChangeNode(const shared_ptr<Node>& node) {
  mNode = node;
  OnRedraw();
}

void Watcher::OnRemovedFromNode() {
  mNode.reset();
}

void Watcher::OnRedraw() {}
void Watcher::OnNameChange() {}
void Watcher::OnSlotGhostChange(Slot* slot) {}
void Watcher::OnSlotConnectionChanged(Slot* slot) {}
void Watcher::OnSlotStructureChanged() {}
void Watcher::OnGraphPositionChanged() {}
void Watcher::OnSplineControlPointsChanged() {}
void Watcher::OnTimeEdited(float time) {}
void Watcher::OnChildNameChange() {}
