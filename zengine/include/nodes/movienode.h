#pragma once

#include "../dom/node.h"
#include "clipnode.h"

class MovieNode: public Node {
public:
  MovieNode();
  virtual ~MovieNode();

  ClipSlot mClips;
};

typedef TypedSlot<NodeType::MOVIE, MovieNode> MovieSlot;
