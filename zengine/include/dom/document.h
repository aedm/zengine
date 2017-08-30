#pragma once

#include "node.h"
#include "graph.h"
#include "../nodes/movienode.h"
#include "../nodes/propertiesnode.h"

class Document: public Node {
public:
  Document();

  GraphSlot mGraphs;
  MovieSlot mMovie;

  PropertiesSlot mProperties;
};
