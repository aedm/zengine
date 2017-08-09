#include <include/shaders/material.h>
#include <include/shaders/engineshaders.h>

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


SolidMaterial::SolidMaterial()
  : Node(NodeType::MATERIAL)
{
  //mMaterial.mShadowPass.Connect(TheEngineShaders->mSolidShadowPass);
}


SolidMaterial::~SolidMaterial() {

}
