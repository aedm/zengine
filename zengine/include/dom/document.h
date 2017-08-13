#pragma once

#include "node.h"
#include "graph.h"
#include "../nodes/movienode.h"

class Document: public Node {
public:
  Document();

  GraphSlot mGraphs;
  MovieSlot mMovie;

  FloatSlot mBPM;
};
