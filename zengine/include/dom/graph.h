#pragma once

#include "node.h"

class Graph: public Node {
public:
  Graph();

  Slot mNodes;

protected:
  virtual void HandleMessage(Message* message) override;

};

typedef TypedSlot<NodeType::GRAPH, Graph> GraphSlot;
