#pragma once

#include "../dom/node.h"
#include "drawable.h"

class SceneNode: public Node {
public:
  SceneNode();
  virtual ~SceneNode();

  DrawableSlot mDrawables;

  void Draw(const Vec2& canvasSize);

protected:
  Globals mGlobals;
};