#pragma once

#include "pass.h"

class Material: public Node {
public:
  Material();

  PassSlot mSolidPass;
  PassSlot mShadowPass;
  PassSlot mZPostPass;
  PassSlot mFluidPaintPass;

  std::shared_ptr<Pass> GetPass(PassType passType);

protected:
  void HandleMessage(Message* message) override;
};

typedef TypedSlot<Material> MaterialSlot;

class SolidMaterial: public Node {
public:
  SolidMaterial();
  virtual ~SolidMaterial();

  std::shared_ptr<Node> GetReferencedNode() override;

protected:
  void HandleMessage(Message* message) override;

private:
  void SetupSlots();

  const std::shared_ptr<Material> mMaterial;
  const std::shared_ptr<Pass> mSolidPass;
  const std::shared_ptr<StubNode> mSolidVertexStub;
  const std::shared_ptr<StubNode> mSolidFragmentStub;

  Slot mGhostSlot;
};