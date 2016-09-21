#pragma once

#include "../dom/node.h"
#include "drawable.h"
#include "cameranode.h"
#include "../render/rendertarget.h"

/// Local time of the current scene
/// TODO: make it not global somehow
extern FloatNode* TheSceneTime;

class SceneNode: public Node {
public:
  SceneNode();
  virtual ~SceneNode();

  DrawableSlot mDrawables;
  CameraSlot mCamera;

  void Draw(RenderTarget* renderTarget);

protected:
  Globals mGlobals;

  void RenderDrawables();

  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};
