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
  virtual void HandleMessage(NodeMessage message, Slot* slot) override;
};

typedef TypedSlot<NodeType::MATERIAL, Material> MaterialSlot;


class SolidMaterial: public Node {
public:
  SolidMaterial();
  virtual ~SolidMaterial();

  virtual Node* GetReferencedNode() override;

protected:
  virtual void HandleMessage(NodeMessage message, Slot* slot) override;

private:
  void SetupSlots();

  Material mMaterial;
  Pass mSolidPass;
  StubNode mSolidVertexStub;
  StubNode mSolidFragmentStub;

  Slot mGhostSlot;
};