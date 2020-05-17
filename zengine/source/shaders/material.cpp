#include <include/shaders/material.h>
#include <include/shaders/engineshaders.h>
#include <include/shaders/enginestubs.h>

REGISTER_NODECLASS(Material, "Material");

Material::Material()
  : mSolidPass(this, "Solid Pass")
  , mShadowPass(this, "Shadow Pass")
  , mZPostPass(this, "Z Postpass")
  , mFluidPaintPass(this, "Fluid Pass")
{}

void Material::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
      EnqueueMessage(MessageType::NEEDS_REDRAW);
      break;
    default: break;
  }
}

std::shared_ptr<Pass> Material::GetPass(PassType passType) {
  switch (passType) {
    case PassType::SHADOW: return mShadowPass.GetNode();
    case PassType::SOLID: return mSolidPass.GetNode();
    case PassType::ZPOST: return mZPostPass.GetNode();
    case PassType::FLUID_PAINT: return mFluidPaintPass.GetNode();
  }
  SHOULD_NOT_HAPPEN;
  return nullptr;
}

REGISTER_NODECLASS(SolidMaterial, "Solid Material");

SolidMaterial::SolidMaterial()
  : mMaterial(std::make_shared<Material>())
  , mSolidPass(std::make_shared<Pass>())
  , mSolidVertexStub(std::make_shared<StubNode>())
  , mSolidFragmentStub(std::make_shared<StubNode>())
  , mGhostSlot(this, "", false, false, false, true)
{
  mMaterial->mShadowPass.Connect(TheEngineShaders->mSolidShadowPass);

  mSolidVertexStub->mSource.Connect(TheEngineStubs->GetStub(
    "material/solid/solidPass-vertex")->mSource.GetDirectNode());
  mSolidFragmentStub->mSource.Connect(TheEngineStubs->GetStub(
    "material/solid/solidPass-fragment")->mSource.GetDirectNode());

  mSolidPass->mVertexStub.Connect(mSolidVertexStub);
  mSolidPass->mFragmentStub.Connect(mSolidFragmentStub);

  mSolidPass->mRenderstate.mDepthTest = true;
  mSolidPass->mBlendModeSlot.SetDefaultValue(1.0f); // normal
  mSolidPass->mFaceModeSlot.SetDefaultValue(0.0f); // front
  mMaterial->mSolidPass.Connect(mSolidPass);

  mGhostSlot.Connect(mMaterial);
  SetupSlots();
}

SolidMaterial::~SolidMaterial() {
  /// TODO: this could possibly be removed
  mMaterial->Dispose();
  mSolidPass->Dispose();
  mSolidVertexStub->Dispose();
  mSolidFragmentStub->Dispose();
}

std::shared_ptr<Node> SolidMaterial::GetReferencedNode() {
  return mMaterial;
}

void SolidMaterial::HandleMessage(Message* message) {
   switch (message->mType) {
     case MessageType::TRANSITIVE_CLOSURE_CHANGED:
       SetupSlots();
       break;
     default: break;
   }
}

void SolidMaterial::SetupSlots() {
  ClearSlots();
  AddSlot(&mGhostSlot);
  
  mSolidFragmentStub->Update();
  for (Slot* slot : mSolidFragmentStub->GetSlots()) {
    if (slot->mIsPublic) {
      AddSlot(slot);
    }
  }
}
