#pragma once

#include "pass.h"

class Material: public Node {
public:
  Material();

  PassSlot mSolidPass;
  PassSlot mShadowPass;
  PassSlot mZPostPass;

  const shared_ptr<Pass> GetPass(PassType passType);

protected:
  virtual void HandleMessage(Message* message) override;
};

typedef TypedSlot<Material> MaterialSlot;

class SolidMaterial: public Node {
public:
  SolidMaterial();
  virtual ~SolidMaterial();

  virtual shared_ptr<Node> GetReferencedNode() override;

protected:
  virtual void HandleMessage(Message* message) override;

private:
  void SetupSlots();

  const shared_ptr<Material> mMaterial;
  const shared_ptr<Pass> mSolidPass;
  const shared_ptr<StubNode> mSolidVertexStub;
  const shared_ptr<StubNode> mSolidFragmentStub;

  Slot mGhostSlot;
};