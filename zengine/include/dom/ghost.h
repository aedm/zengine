#pragma once

#include "node.h"
#include <vector>
#include <map>

using namespace std;

class Ghost : public Node {
public:
  Ghost(const shared_ptr<Node>& originalNode);

  Slot mOriginalNode;

  virtual bool IsGhostNode() override;

protected:
  set<shared_ptr<Node>> mInternalNodes;
  map<shared_ptr<Node>, shared_ptr<Node>> mNodeMapping;

  shared_ptr<Node> mMainInternalNode;
  

  virtual shared_ptr<Node> GetReferencedNode() override;

  void Regenerate();

private:
  
};