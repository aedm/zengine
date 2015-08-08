#pragma once

#include "../dom/node.h"
#include "drawable.h"
#include "camera.h"

class SceneNode: public Node {
public:
  SceneNode();
  virtual ~SceneNode();

  DrawableSlot mDrawables;
  CameraSlot mCamera;

  void Draw(const Vec2& canvasSize);

protected:
  Globals mGlobals;
};