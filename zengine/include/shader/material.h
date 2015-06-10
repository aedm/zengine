#pragma once

#include "pass.h"
#include "../shaders/shaders.h"

class Material: public Node {
public:
  Material();
  virtual ~Material();

  Slot mSolidPass;

  Pass* GetPass();

protected:
  virtual void HandleMessage(Slot* slot, NodeMessage message, const void* payload);
};