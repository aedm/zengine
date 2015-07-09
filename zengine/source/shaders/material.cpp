#include <include/shaders/material.h>

REGISTER_NODECLASS(Material, "Material");

static SharedString SolidPassSlotName = make_shared<string>("Solid Pass");

Material::Material()
  : Node(NodeType::MATERIAL)
  , mSolidPass(this, SolidPassSlotName) {}


Material::~Material() {}

void Material::HandleMessage(Slot* slot, NodeMessage message, const void* payload) {}

Pass* Material::GetPass() {
  return mSolidPass.GetNode();
}
