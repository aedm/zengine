#include <include/shaders/material.h>

REGISTER_NODECLASS(Material, "Material");

static SharedString SolidPassSlotName = make_shared<string>("Solid Pass");

Material::Material()
  : Node(NodeType::MATERIAL)
  , mSolidPass(this, SolidPassSlotName) {}


Material::~Material() {}

void Material::HandleMessage(NodeMessage message, Slot* slot, void* payload) {
  switch (message) {
    case NodeMessage::NEEDS_REDRAW:
      SendMsg(NodeMessage::NEEDS_REDRAW);
      break;
    default: break;
  }
}

Pass* Material::GetPass() {
  return mSolidPass.GetNode();
}
