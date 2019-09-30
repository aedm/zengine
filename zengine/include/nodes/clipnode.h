#pragma once

#include "../dom/node.h"
#include "scenenode.h"

class ClipNode: public Node {
public:
  ClipNode();

  SceneSlot mSceneSlot;
  FloatSlot mStartTime;
  FloatSlot mLength;
  FloatSlot mTrackNumber;
  FloatSlot mFakeStartTime;

  FloatSlot mClearColorBuffer;
  FloatSlot mClearDepthBuffer;
  FloatSlot mCopyToSecondaryBuffer;
  FloatSlot mTargetSquareBuffer;

  FloatSlot mApplyPostprocessBefore;

  void Draw(RenderTarget* renderTarget, Globals* globals, float clipTime);

protected:
  void HandleMessage(Message* message) override;
};

typedef TypedSlot<ClipNode> ClipSlot;
