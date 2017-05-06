#pragma once

#include "../dom/node.h"
#include "scenenode.h"

class ClipNode: public Node {
public:
  ClipNode();
  virtual ~ClipNode();

  SceneSlot mSceneSlot;
  FloatSlot mStartTime;
  FloatSlot mLength;
  FloatSlot mTrackNumber;

protected:
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;

};

typedef TypedSlot<NodeType::CLIP, ClipNode> ClipSlot;
