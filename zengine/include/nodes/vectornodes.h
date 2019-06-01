#pragma once

#include "valuenodes.h"

class FloatsToVec3Node: public ValueNode<Vec3> {
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

class FloatsToVec4Node : public ValueNode<Vec4> {
public:
  FloatsToVec4Node();

  FloatSlot mX;
  FloatSlot mY;
  FloatSlot mZ;
  FloatSlot mW;

  virtual const Vec4& Get() override;

  virtual void HandleMessage(Message* message) override;

protected:
  virtual void Operate() override;

  Vec4 mValue;
};


class FloatToFloatNode : public ValueNode<float> {
public:
  FloatToFloatNode();

  FloatSlot mX;

  virtual const float& Get() override;

  virtual void HandleMessage(Message* message) override;

protected:
  virtual void Operate() override;

  float mValue;
};

class MaddNode : public ValueNode<float> {
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
