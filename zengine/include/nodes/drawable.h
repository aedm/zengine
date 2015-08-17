#pragma once

#include "meshnode.h"
#include "../shaders/material.h"

class Drawable: public Node {
public:
  Drawable();
  virtual ~Drawable();

  MeshSlot mMesh;
  MaterialSlot mMaterial;

  Vec3Slot mMove;
  Vec3Slot mRotate;

  void Draw(Globals* globals, PrimitiveTypeEnum Primitive = PRIMITIVE_TRIANGLES);

protected:
  /// Handle received messages
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};

typedef TypedSlot<NodeType::DRAWABLE, Drawable> DrawableSlot;
