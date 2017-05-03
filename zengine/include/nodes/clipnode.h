#pragma once

#include "../dom/node.h"
#include "scenenode.h"

class ClipNode: public Node {
public:
  ClipNode();
  virtual ~ClipNode();

  SceneSlot mSceneSlot;
  FloatSlot mStartTime;
  FloatSlot mEndTime;
  FloatSlot mTrackNumber;
};

typedef TypedSlot<NodeType::CLIP, ClipNode> ClipSlot;
