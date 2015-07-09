#pragma once

#include "pass.h"

class Material: public Node {
public:
  Material();
  virtual ~Material();

  PassSlot mSolidPass;

  Pass* GetPass();

protected:
  virtual void HandleMessage(Slot* slot, NodeMessage message, const void* payload);
};

typedef TypedSlot<NodeType::MATERIAL, Material> MaterialSlot;
