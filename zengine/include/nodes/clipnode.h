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

  FloatSlot mClearColorBuffer;
  FloatSlot mClearDepthBuffer;
  FloatSlot mCopyToSecondaryBuffer;

  FloatSlot mApplyPostprocessBefore;

  void Draw(RenderTarget* renderTarget, Globals* globals, float clipTime);

protected:
  virtual void HandleMessage(Message* message) override;

};

typedef TypedSlot<ClipNode> ClipSlot;
