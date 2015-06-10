#pragma once

#include "material.h"
#include "../nodes/meshnode.h"
#include "../shaders/shaders.h"

class Drawable: public Node {
public:
  Drawable();
  virtual ~Drawable();

  MeshSlot mMesh;
  Slot mMaterial;

  void Draw(Globals* globals);
};