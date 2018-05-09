#pragma once

#include "node.h"
#include <vector>
#include <map>

using namespace std;

class Ghost : public Node {
public:
  Ghost();

  Slot mOriginalNode;

  virtual bool IsGhostNode() override;
  
  /// Returns true if ghost node is simply a reference without any
  /// internal nodes.
  bool IsDirectReference();

protected:
  set<shared_ptr<Node>> mInternalNodes;
  map<shared_ptr<Node>, shared_ptr<Node>> mNodeMapping;

  Slot mMainInternalNode;
  
  virtual void HandleMessage(Message* message) override;

  virtual shared_ptr<Node> GetReferencedNode() override;

  void Regenerate();

private:
  
};