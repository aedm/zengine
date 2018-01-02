#pragma once

#include "node.h"

/// Ghost nodes are references / implementations / clones of other nodes.

class Ghost : public Node {
public:
  Ghost();
  virtual ~Ghost();

protected:
  vector<Node*> mInternalNodes;

  
};
