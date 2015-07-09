#pragma once

#include "meshnode.h"
#include "../shaders/material.h"

class Drawable: public Node {
public:
  Drawable();
  virtual ~Drawable();

  MeshSlot mMesh;
  MaterialSlot mMaterial;

  void Draw(Globals* globals, PrimitiveTypeEnum Primitive = PRIMITIVE_TRIANGLES);
};