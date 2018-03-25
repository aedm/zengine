#include <include/dom/ghost.h>
#include <include/dom/nodetype.h>

Ghost::Ghost(Node* originalNode)
  : Node()
  , mOriginalNode(this, nullptr, false, false, true, false)
{
  mOriginalNode.Connect(originalNode);
  Regenerate();
}

Ghost::~Ghost() {
  SafeDelete(mMainInternalNode);
}

bool Ghost::IsGhostNode() {
  return true;
}

Node* Ghost::GetReferencedNode() {
  return mMainInternalNode;
}

void Ghost::Regenerate() {
  Node* original = mOriginalNode.GetReferencedNode();
  mMainInternalNode = NodeRegistry::GetInstance()->GetNodeClass(original)->Manufacture();
}

