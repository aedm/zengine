#pragma once

#include "../dom/node.h"
#include "drawable.h"
#include "cameranode.h"

class SceneNode: public Node {
public:
  SceneNode();
  virtual ~SceneNode();

  DrawableSlot mDrawables;
  CameraSlot mCamera;

  void Draw(const Vec2& canvasSize);

protected:
  Globals mGlobals;

  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};