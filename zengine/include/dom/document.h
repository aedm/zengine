#pragma once

#include "node.h"
#include "graph.h"

class Document: public Node {
public:
  Document();

  GraphSlot mGraphs;
};
