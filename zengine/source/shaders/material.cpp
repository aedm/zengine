#include <include/shaders/material.h>
#include <include/shaders/engineshaders.h>
#include <include/shaders/enginestubs.h>

REGISTER_NODECLASS(Material, "Material");

static SharedString SolidPassSlotName = make_shared<string>("Solid Pass");
static SharedString ShadowPassSlotName = make_shared<string>("Shadow Pass");

Material::Material()
  : Node(NodeType::MATERIAL)
  , mSolidPass(this, SolidPassSlotName)
  , mShadowPass(this, ShadowPassSlotName) {}


Material::~Material() {}


void Material::HandleMessage(Message* message) {
  switch (message->mType) {
    case MessageType::SLOT_CONNECTION_CHANGED:
      TheMessageQueue.Enqueue(nullptr, this, MessageType::NEEDS_REDRAW);
      break;
    default: break;
  }
}


Pass* Material::GetPass(PassType passType) {
  switch (passType) {
    case PassType::SHADOW: return mShadowPass.GetNode();
    case PassType::SOLID: return mSolidPass.GetNode();
  }
  SHOULD_NOT_HAPPEN;
  return nullptr;
}


REGISTER_NODECLASS(SolidMaterial, "Solid Material");


SolidMaterial::SolidMaterial()
  : Node(NodeType::MATERIAL)
  , mGhostSlot(NodeType::ALLOW_ALL, this, nullptr, false, false, false, true)
{
  mMaterial.mShadowPass.Connect(&TheEngineShaders->mSolidShadowPass);

  mSolidVertexStub.mSource.Connect(
    TheEngineStubs->GetStub("material/solid/solidPass-vertex")->mSource.GetDirectNode());
  mSolidFragmentStub.mSource.Connect(
    TheEngineStubs->GetStub("material/solid/solidPass-fragment")->mSource.GetDirectNode());

  mSolidPass.mVertexStub.Connect(&mSolidVertexStub);
  mSolidPass.mFragmentStub.Connect(&mSolidFragmentStub);

  mSolidPass.mRenderstate.mDepthTest = true;
  mSolidPass.mBlendModeSlot.SetDefaultValue(1.0f); // normal
  mSolidPass.mFaceModeSlot.SetDefaultValue(0.0f); // front
  mMaterial.mSolidPass.Connect(&mSolidPass);

  mGhostSlot.Connect(&mMaterial);
  SetupSlots();
}


SolidMaterial::~SolidMaterial() {
}


Node* SolidMaterial::GetReferencedNode() {
  return &mMaterial;
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
  AddSlot(&mGhostSlot, false, false, false);

  for (Slot* slot : mSolidFragmentStub.GetPublicSlots()) {
    AddSlot(slot, true, true, true);
  }
}
