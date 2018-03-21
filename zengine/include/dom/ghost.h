#pragma once

#include "node.h"

class Ghost : public Node {
public:
  Ghost();

  void SetOriginalNode(Node* originalNode);

protected:
  vector<Node*> mInternalNodes;
  Node* mMainInternalNode;

  Node* mOriginalNode;

  void Regenerate();

private:
  
};