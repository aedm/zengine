#pragma once

#include "node.h"

class Graph: public Node {
public:
  Graph();

  Slot mNodes;
};

typedef TypedSlot<NodeType::GRAPH, Graph> GraphSlot;
