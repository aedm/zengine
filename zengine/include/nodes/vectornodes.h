#pragma once

#include "valuenodes.h"

class FloatsToVec3Node: public ValueNode<vec3> {
public:
  FloatsToVec3Node();

  FloatSlot mX;
  FloatSlot mY;
  FloatSlot mZ;

  const vec3& Get() override;

  void HandleMessage(Message* message) override;

protected:
  void Operate() override;

  vec3 mValue;
};

class FloatsToVec4Node : public ValueNode<vec4> {
public:
  FloatsToVec4Node();

  FloatSlot mX;
  FloatSlot mY;
  FloatSlot mZ;
  FloatSlot mW;

  const vec4& Get() override;

  void HandleMessage(Message* message) override;

protected:
  void Operate() override;

  vec4 mValue;
};


class FloatToFloatNode : public ValueNode<float> {
public:
  FloatToFloatNode();

  FloatSlot mX;

  const float& Get() override;

  void HandleMessage(Message* message) override;

protected:
  void Operate() override;

  float mValue;
};

class MaddNode : public ValueNode<float> {
public:
  MaddNode();

  FloatSlot mA;
  FloatSlot mB;
  FloatSlot mC;

  const float& Get() override;

  void HandleMessage(Message* message) override;

protected:
  void Operate() override;

  float mValue;
};
