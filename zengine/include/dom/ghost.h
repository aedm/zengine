#pragma once

#include "node.h"
#include <map>

class Ghost : public Node {
public:
  Ghost();

  Slot mOriginalNode;

  bool IsGhostNode() override;
  
  /// Returns true if ghost node is simply a reference without any
  /// internal nodes.
  bool IsDirectReference() const;

protected:
  std::set<std::shared_ptr<Node>> mInternalNodes;
  std::map<std::shared_ptr<Node>, std::shared_ptr<Node>> mNodeMapping;

  Slot mMainInternalNode;

  void HandleMessage(Message* message) override;

  std::shared_ptr<Node> GetReferencedNode() override;

  void Regenerate();
};