#pragma once

#include "meshnode.h"
#include "../shaders/material.h"
#include "../shaders/pass.h"

class Drawable;
typedef TypedSlot<NodeType::DRAWABLE, Drawable> DrawableSlot;

class Drawable: public Node {
public:
  Drawable();
  virtual ~Drawable();

  MeshSlot mMesh;
  MaterialSlot mMaterial;

  DrawableSlot mChildren;
  Vec3Slot mMove;
  Vec3Slot mRotate;
  FloatSlot mInstances;

  void Draw(Globals* globals, PassType passType, PrimitiveTypeEnum Primitive = PRIMITIVE_TRIANGLES);

protected:
  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};

