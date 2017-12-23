#pragma once

#include "valuenodes.h"

class FloatsToVec3Node: public ValueNode<ValueType::VEC3> {
public:
  FloatsToVec3Node();
  virtual ~FloatsToVec3Node();

  FloatSlot mX;
  FloatSlot mY;
  FloatSlot mZ;

  virtual const Vec3& Get() override;

  virtual void HandleMessage(Message* message) override;

protected:
  virtual void Operate() override;

  Vec3 mValue;
};