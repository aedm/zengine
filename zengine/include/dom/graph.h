#pragma once

#include "node.h"

class Graph: public Node {
public:
  Graph();

  Slot mNodes;

protected:
  void HandleMessage(Message* message) override;

};

typedef TypedSlot<Graph> GraphSlot;
