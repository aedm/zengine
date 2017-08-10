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


void Material::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::SLOT_CONNECTION_CHANGED:
      ReceiveMessage(NodeMessage::NEEDS_REDRAW);
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
  , mGhostSlot(NodeType::ALLOW_ALL, this, nullptr, false, false, false, false)
{
  mMaterial.mShadowPass.Connect(&TheEngineShaders->mSolidShadowPass);

  mSolidPass.mVertexStub.Connect(
    TheEngineStubs->GetStub("material/solid/solidPass-vertex"));
  mSolidPass.mFragmentStub.Connect(
    TheEngineStubs->GetStub("material/solid/solidPass-fragment"));

  mSolidPass.mRenderstate.mDepthTest = true;
  mSolidPass.mBlendModeSlot.SetDefaultValue(1.0f); // normal
  mSolidPass.mFaceModeSlot.SetDefaultValue(0.0f); // front
  mMaterial.mSolidPass.Connect(&mSolidPass);

  mGhostSlot.Connect(&mMaterial);

  for (Slot* slot : mSolidPass.mFragmentStub.GetNode()->GetPublicSlots()) {
    AddSlot(slot, true, true, true);
  }
}


SolidMaterial::~SolidMaterial() {
}


Node* SolidMaterial::GetReferencedNode() {
  return &mMaterial;
}

void SolidMaterial::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
//   switch (message) {
//     case NodeMessage::SLOT_CONNECTION_CHANGED:
//     case NodeMessage::VALUE_CHANGED:
//       ReceiveMessage(NodeMessage::NEEDS_REDRAW);
//       break;
//     default: break;
//   }
}
