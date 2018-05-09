#pragma once

#include "valuenodes.h"

class FloatsToVec3Node: public ValueNode<ValueType::VEC3> {
public:
  FloatsToVec3Node();

  FloatSlot mX;
  FloatSlot mY;
  FloatSlot mZ;

  virtual const Vec3& Get() override;

  virtual void HandleMessage(Message* message) override;

protected:
  virtual void Operate() override;

  Vec3 mValue;
};

class FloatToFloatNode : public ValueNode<ValueType::FLOAT> {
public:
  FloatToFloatNode();

  FloatSlot mX;

  virtual const float& Get() override;

  virtual void HandleMessage(Message* message) override;

protected:
  virtual void Operate() override;

  float mValue;
};

class MaddNode : public ValueNode<ValueType::FLOAT> {
public:
  MaddNode();

  FloatSlot mA;
  FloatSlot mB;
  FloatSlot mC;

  virtual const float& Get() override;

  virtual void HandleMessage(Message* message) override;

protected:
  virtual void Operate() override;

  float mValue;
};
