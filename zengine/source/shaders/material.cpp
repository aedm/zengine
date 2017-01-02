#include <include/shaders/material.h>

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
    case NodeMessage::NEEDS_REDRAW:
      SendMsg(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}


Pass* Material::GetPass(PassType passType) {
  switch (passType) {
    case PassType::SHADOW: return mShadowPass.GetNode();
    case PassType::SOLID: return mSolidPass.GetNode();
  }
  SHOULDNT_HAPPEN;
  return nullptr;
}
