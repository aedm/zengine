#pragma once

#include "node.h"

class Document: public Node {
public:
  Document();

  Slot mGraphs;
};
