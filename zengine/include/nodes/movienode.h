#pragma once

#include "../dom/node.h"
#include "clipnode.h"

class MovieNode: public Node {
public:
  MovieNode();
  virtual ~MovieNode();

  ClipSlot mClips;

  void Draw(RenderTarget* renderTarget);
};

typedef TypedSlot<NodeType::MOVIE, MovieNode> MovieSlot;
