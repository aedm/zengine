#pragma once

#include "pass.h"

class Material: public Node {
public:
  Material();
  virtual ~Material();

  PassSlot mSolidPass;
  PassSlot mShadowPass;

  Pass* GetPass(PassType passType);

protected:
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;
};

typedef TypedSlot<NodeType::MATERIAL, Material> MaterialSlot;


class SolidMaterial: public Node {
public:
  SolidMaterial();
  virtual ~SolidMaterial();

  virtual Node* GetReferencedNode() override;

protected:
  virtual void HandleMessage(NodeMessage message, Slot* slot, void* payload) override;

private:
  Material mMaterial;
  Pass mSolidPass;

  Slot mGhostSlot;
};