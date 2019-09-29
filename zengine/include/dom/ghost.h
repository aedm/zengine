#pragma once

#include "node.h"
#include <map>

using namespace std;

class Ghost : public Node {
public:
  Ghost();

  Slot mOriginalNode;

  bool IsGhostNode() override;
  
  /// Returns true if ghost node is simply a reference without any
  /// internal nodes.
  bool IsDirectReference() const;

protected:
  set<shared_ptr<Node>> mInternalNodes;
  map<shared_ptr<Node>, shared_ptr<Node>> mNodeMapping;

  Slot mMainInternalNode;

  void HandleMessage(Message* message) override;

  shared_ptr<Node> GetReferencedNode() override;

  void Regenerate();
};