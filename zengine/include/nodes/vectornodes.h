#pragma once

#include "valuenodes.h"

class FloatsToVec3Node: public ValueNode<NodeType::VEC3> {
public:
  FloatsToVec3Node();
  virtual ~FloatsToVec3Node();

  FloatSlot mX;
  FloatSlot mY;
  FloatSlot mZ;

  virtual const Vec3& Get() override;

  virtual void HandleMessage(NodeMessage message, Slot* slot) override;

protected:
  virtual void Operate() override;

  Vec3 mValue;
};