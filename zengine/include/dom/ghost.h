#pragma once

#include "node.h"

class Ghost : public Node {
public:
  Ghost(Node* originalNode);
  virtual ~Ghost();

  Slot mOriginalNode;

  virtual bool IsGhostNode() override;

protected:
  vector<Node*> mInternalNodes;
  Node* mMainInternalNode = nullptr;

  virtual Node* GetReferencedNode() override;

  void Regenerate();

private:
  
};